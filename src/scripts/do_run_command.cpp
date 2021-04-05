
#include "do_run_command.h"
#include <wsjcpp_core.h>
#include <mutex>
#include <sstream>
#include <iostream>
#include <vector>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <sys/wait.h>
#include <sysexits.h>
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <errno.h>
// ----------------------------------------------------------------------
// DoRunCommand

DoRunCommand::DoRunCommand(
    const std::string &sDir,
    const std::vector<std::string> &vArgs,
    int nUserId,
    int nGroupId
) {
    TAG = "DoRunCommand";
    m_sDir = sDir;
    m_vArgs = vArgs;
    m_nUserId = nUserId;
    m_nGroupId = nGroupId;
    m_sCommand = WsjcppCore::join(vArgs, " ");
    // TODO will be not work some 'cd ..'
}

bool DoRunCommand::hasError() {
    return m_bHasError;
}

// ----------------------------------------------------------------------

int DoRunCommand::exitCode() {
    return m_nExitCode;
}

// ----------------------------------------------------------------------

bool DoRunCommand::isTimeout() {
    return m_bFinishedByTimeout;
}

// ----------------------------------------------------------------------

const std::string &DoRunCommand::outputString() {
    return m_sOutput;
}

// ----------------------------------------------------------------------

void* newProcessThread(void *arg) {
    DoRunCommand *m_pDoRunCommand = (DoRunCommand *)arg;
    pthread_detach(pthread_self());
    m_pDoRunCommand->run();
    return 0;
}

// ----------------------------------------------------------------------

void DoRunCommand::start(int nTimeoutMS) {
    m_bFinished = false;
    m_bFinishedByTimeout = false;
    m_nTimeoutMS = nTimeoutMS;
    int nTimeWait = 0;
    int nSleepMS = 100;

    pthread_create(&m_pProcessThread, NULL, &newProcessThread, (void *)this);

    while (nTimeWait < m_nTimeoutMS && !m_bFinished) {
        WsjcppLog::info(TAG, "Wait: " + std::to_string(nSleepMS) + "ms");
        std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMS));
        nTimeWait = nTimeWait + nSleepMS;
    }
    if (!m_bFinished) {
        m_bHasError = true;
        m_bFinishedByTimeout = true;
        m_nExitCode = -1;
        m_sOutput = "timeout";
        // pthread_kill(m_pProcessThread, -9);

        if (m_nPid > 0) {
            kill(m_nPid, 9);
            // kill(m_nPid, 9);
        }
        pthread_cancel(m_pProcessThread);
    }
}

void DoRunCommand::run() {
    m_nExitCode = 1;
    m_sOutput = "";
    m_bHasError = false;
    m_bFinishedByTimeout = false;
    m_bFinished = false;
    m_nPid = 0;

    int fd[2];

    if (pipe(fd) != 0) {
        m_sOutput = "Could not open pipe";
        WsjcppLog::err(TAG, m_sOutput);
        m_nExitCode = -1;
        m_bHasError = true;
        m_bFinishedByTimeout = false;
        m_bFinished = true;
        return;
    }

    WsjcppLog::info(TAG, "Will be change dir to: '" + m_sDir + "'");
    int nSize = m_vArgs.size();
    WsjcppLog::info(TAG, "Exec command: {" + m_sCommand + "}");
    for (int i = 0; i < nSize; i++) {
        WsjcppLog::info(TAG, "Exec arg" + std::to_string(i)+ ": {" + m_vArgs[i] + "}");
    }

    fflush(stdout);
    fflush(stderr);

    pid_t nChildPid = fork();

    if(nChildPid < 0) {
        m_sOutput = "fork failed!";
        WsjcppLog::err(TAG, m_sOutput);
        m_nExitCode = -1;
        m_bHasError = true;
        m_bFinishedByTimeout = false;
        m_bFinished = true;
        return;
    } else if (nChildPid == 0) {
        // child process
        if (dup2(fd[1], STDOUT_FILENO) < 0) { // redirect from pipe to stdout
            perror("dup2");
            exit(-1);
        }
        if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) { // redirects stderr to stdout below this line.
            perror("dup2");
            exit(-1);
        }
        close(fd[0]);
        close(fd[1]);
        printf("fork: Child process id=%d\n", getpid());
        printf("fork: Change dir '%s'\n", m_sDir.c_str());
        chdir(m_sDir.c_str());

        printf("fork: current groupid '%d'\n", getgid());
        if (getgid() != m_nGroupId) {
            printf("fork: Switch to groupid '%d'\n", m_nGroupId);
            if (setgid(m_nGroupId) != 0) {
                perror("Could not change run as user (setgid)\n");
                exit(-1);
            }
        }
        
        printf("fork: current userid '%d'\n", getuid());
        if (getuid() != m_nUserId) {
            printf("fork: Switch to userid '%d'\n", m_nUserId);
            if (setuid(m_nUserId) != 0) {
                perror("Could not change run as user (setuid)\n");
                exit(-1);
            }
        }
        
        

        // setpgid(nChildPid, nChildPid); //Needed so negative PIDs can kill children of /bin/sh

        char **pArgs = new char * [nSize + 1];
        pArgs[nSize] = (char *) 0;
        pArgs[0] = new char[m_vArgs[0].length() + 1];
        for (int n = 0; n < nSize; n++) {
            int nLen = m_vArgs[n].length();
            pArgs[n] = new char[nLen + 1];
            std::memcpy(pArgs[n], m_vArgs[n].c_str(), nLen);
            pArgs[n][nLen] = 0;
        }
        
        // TODO after exec delete from memory
        execvp(
            pArgs[0], // 
            pArgs // first argument must be same like executable file
            // (char *) 0
        );
        perror("execvp");
        exit(-1);
    }
    
    // parent process
    WsjcppLog::info(TAG, "Parent process id=" + std::to_string(getpid()));
    // setpgid(nChildPid, ::getpid());
    close(fd[1]);
    int nPipeOut = fd[0];
    m_nPid = nChildPid;

    m_sOutput = "";
    const int nSizeBuffer = 4096;
    char buffer[nSizeBuffer];
    std::memset(&buffer, 0, nSizeBuffer);
    try {
        while (read(nPipeOut, buffer, nSizeBuffer-1) > 0) {
            m_sOutput += std::string(buffer);
            std::memset(&buffer, 0, nSizeBuffer);
        }
        int status;
        if (waitpid(m_nPid, &status, 0) == -1) {
            perror("waitpid() failed");
            exit(EXIT_FAILURE);
        }
        if (WIFEXITED(status)) {
            m_bHasError = false;
            m_nExitCode = WEXITSTATUS(status);
        }
    } catch (std::bad_alloc& ba) {
        close(nPipeOut);
        m_bHasError = true;
        m_nExitCode = -1;
        std::cerr << "bad alloc" << std::endl;
        return;
    }
    
    close(nPipeOut);

    if (m_bFinishedByTimeout) {
        return;
    }

    if (m_nExitCode < 0) { // basic concept of errors
        m_bHasError = true;
    }

    // look here https://shapeshed.com/unix-exit-codes/
    if (m_nExitCode == 1) { // Catchall for general errors
        m_bHasError = true;
    }

    if (m_nExitCode == 2) { // Misuse of shell builtins (according to Bash documentation)
        m_bHasError = true;
    }

    if (m_nExitCode == 126) { // Command invoked cannot execute
        m_bHasError = true;
    }

    if (m_nExitCode == 127) { // “command not found”
        m_bHasError = true;
    }

    if (m_nExitCode == 128) { // Invalid argument to exit
        m_bHasError = true;
    }

    if (m_nExitCode > 128 && m_nExitCode < 140) { // Fatal error signal “n”
        // 130 - Script terminated by Control-C
        m_bHasError = true;
    }
    
    if (m_bHasError) {
        m_nExitCode = -1;
        // Log::err(TAG, m_sOutput);
    } else {
        // Log::info(TAG, "DEBUG output: " + m_sOutput);
    }
    m_bFinished = true;
    // Log::info(TAG, "Finished");
}

// ----------------------------------------------------------------------

std::string DoRunCommand::exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}
