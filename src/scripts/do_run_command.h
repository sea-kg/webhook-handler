#ifndef DO_RUN_COMMAND_H
#define DO_RUN_COMMAND_H

#include <string>
#include <stdio.h>
#include <iostream>
#include <mutex>
#include <deque>
#include <vector>

class DoRunCommand {
    public:
        DoRunCommand(
            const std::string &sDir,
            const std::vector<std::string> &vArgs
        );
        void start(int nTimeoutMS);
        bool hasError();
        int exitCode();
        bool isTimeout();
        const std::string &outputString();
        void run();

    private:
        std::string exec(const char* cmd);
        
        std::string TAG;
        std::string m_sDir;
        std::vector<std::string> m_vArgs;
        std::string m_sCommand;
        int m_nTimeoutMS;
        pid_t m_nPid;
        pthread_t m_pProcessThread;
        int m_nExitCode;
        bool m_bHasError;
        bool m_bFinished;
        bool m_bFinishedByTimeout;
        std::string m_sOutput;
};


#endif // DO_RUN_COMMAND_H