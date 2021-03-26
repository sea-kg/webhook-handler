#ifndef SCRIPTS_THREAD_H
#define SCRIPTS_THREAD_H

#include <webhook_handler_config.h>
#include <deque_webhooks.h>

class ScriptsThread {
    public:
        ScriptsThread(WebhookHandlerConfig *pConfig, int nWaitSecondsBetweenRunScripts, int nNumber, DequeWebhooks *pDequeWebhooks);
        void start();
        void run();

    private:
        std::string TAG;
        pthread_t m_checkerThread;
        WebhookHandlerConfig *m_pConfig;
        int m_nWaitSecondsBetweenRunScripts;
        DequeWebhooks *m_pDequeWebhooks;
        int m_nNumber;
};

#endif // SCRIPTS_THREAD_H
