
#include "do_run_commands.h"
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
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <sys/wait.h>
#include <sysexits.h>
#include <iostream>
#include <cstring>

// ----------------------------------------------------------------------
// DoRunCommands

DoRunCommands::DoRunCommands(
    const std::string &sDir,
    const std::vector<std::string> &vCommands
) {
    TAG = "DoRunCommands";
    m_sDir = sDir;
    m_vCommands = vCommands;
}

bool DoRunCommands::hasError() {
    return m_bHasError;
}

// ----------------------------------------------------------------------

int DoRunCommands::exitCode() {
    return m_nExitCode;
}

// ----------------------------------------------------------------------

bool DoRunCommands::isTimeout() {
    return m_bFinishedByTimeout;
}

// ----------------------------------------------------------------------

const std::string &DoRunCommands::outputString() {
    return m_sOutput;
}

// ----------------------------------------------------------------------

void* newProcessThread(void *arg) {
    // Log::info("newRequest", "");
    DoRunCommands *m_pDoRunCommands = (DoRunCommands *)arg;
    pthread_detach(pthread_self());
    m_pDoRunCommands->run();
    return 0;
}

// ----------------------------------------------------------------------

void DoRunCommands::start(int nTimeoutMS) {
    m_bFinished = false;
    m_bFinishedByTimeout = false;
    m_nTimeoutMS = nTimeoutMS;
    int nTimeWait = 0;
    int nSleepMS = 100;

    pthread_create(&m_pProcessThread, NULL, &newProcessThread, (void *)this);

    while (nTimeWait < m_nTimeoutMS && !m_bFinished) {
        // Log::info(TAG, "Wait: " + std::to_string(nTimeWait) + "ms");
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

// ----------------------------------------------------------------------

void DoRunCommands::run() {
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
        dup2 (fd[1], STDOUT_FILENO);
        dup2(1, 2); // redirects stderr to stdout below this line.
        close(fd[0]);
        close(fd[1]);
        chdir(m_sDir.c_str());
        WsjcppLog::info(TAG, "Change Dir: " + m_sDir);
        // setpgid(nChildPid, nChildPid); //Needed so negative PIDs can kill children of /bin/sh
        for (int i = 0; i < m_vCommands.size(); i++) {
            std::string sCommand = m_vCommands[i];
            WsjcppLog::info(TAG, "Run command: '" + sCommand + "'");
            std::vector<std::string> vArgs = DoRunCommands::parseCommands(sCommand);
            int nSize = vArgs.size();
            char **pArgs = new char * [nSize + 1];
            pArgs[nSize] = NULL;
            pArgs[0] = new char[vArgs[0].length() + 1];
            for (int n = 0; n < nSize; n++) {
                int nLen = vArgs[n].length();
                pArgs[n] = new char[nLen + 1];
                std::memcpy(pArgs[n], vArgs[n].c_str(), nLen);
                pArgs[n][nLen] = 0;
            }
            execvp(
                vArgs[0].c_str(), // 
                pArgs // first argument must be same like executable file
                // (char *) 0
            );
            perror("execvp");
            exit(-1);
        }
    }
    
    // parent process;
    // setpgid(nChildPid, ::getpid());
    close(fd[1]);
    int nPipeOut = fd[0];
    m_nPid = nChildPid;
    // Log::info(TAG, "PID: " + std::to_string(m_nPid));

    m_sOutput = "";
    const int nSizeBuffer = 4096;
    char buffer[nSizeBuffer];
    std::memset(&buffer, 0, nSizeBuffer);
    try {
        int nbytes = read(nPipeOut, buffer, nSizeBuffer-1);
        m_sOutput += std::string(buffer);
        int status;
        if ( waitpid(m_nPid, &status, 0) == -1 ) {
            perror("waitpid() failed");
            exit(EXIT_FAILURE);
        }

        if ( WIFEXITED(status) ) {
            m_bHasError = false;
            m_nExitCode = WEXITSTATUS(status);
        }
    } catch (std::bad_alloc& ba) {
        close(nPipeOut);
        m_bHasError = true;
        m_nExitCode = -1;
        WsjcppLog::err("DoRunProcess", "bad alloc");
        return;
    }
    
    close(nPipeOut);

    // Log::info(TAG, "Process exit code: " + std::to_string(m_nExitCode));

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

std::vector<std::string> DoRunCommands::parseCommands(const std::string& sCommands) {
    std::string sToken = "";
    std::vector<std::string> sArgs;
    int nState = 0;
    for (int i = 0; i < sCommands.length(); i++) {
        if (nState == 0 && sCommands[i] == '"') {
            sToken += sCommands[i];
            nState = 1; // string double quote
        } else if (nState == 1 && sCommands[i] != '"') {
            sToken += sCommands[i];
        } else if (nState == 1 && sCommands[i] == '"') {
            sToken += sCommands[i];
            nState = 0; // end string double quote
        } else if (nState == 0 && sCommands[i] == '\'') {
            sToken += sCommands[i];
            nState = 2; // string single quote
        } else if (nState == 2 && sCommands[i] != '\'') {
            sToken += sCommands[i];
        } else if (nState == 2 && sCommands[i] == '\'') {
            sToken += sCommands[i];
            nState = 0; // end string double quote
        } else if (nState == 0 && sCommands[i] != ' ') {
            sToken += sCommands[i];
        } else if (nState == 0 && sCommands[i] == ' ') {
            sToken = WsjcppCore::trim(sToken);
            if (sToken != "") {
                sArgs.push_back(sToken);
                sToken = "";
            }
        }
    }
    sToken = WsjcppCore::trim(sToken);
    if (sToken != "") {
        sArgs.push_back(sToken);
    }
    // = WsjcppCore::split(sCommands, " ");
    return sArgs;
}