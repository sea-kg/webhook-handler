#ifndef SCRIPTS_THREAD_H
#define SCRIPTS_THREAD_H

#include <config.h>
#include <deque_webhooks.h>

class ScriptsThread {
    public:
        ScriptsThread(Config *pConfig, int nWaitSecondsBetweenRunScripts, int nNumber, DequeWebhooks *pDequeWebhooks);
        void start();
        void run();

    private:
        std::string TAG;
        pthread_t m_checkerThread;
        Config *m_pConfig;
        int m_nWaitSecondsBetweenRunScripts;
        DequeWebhooks *m_pDequeWebhooks;
        int m_nNumber;

        int runScript(const std::string &sCommand);
};

#endif // SCRIPTS_THREAD_H
