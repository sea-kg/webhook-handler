#ifndef DEQUE_WEBHOOKS_H
#define DEQUE_WEBHOOKS_H

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mutex>
#include <deque>

class WebhookRequest {
    public:
        WebhookRequest();
        WebhookRequest(
            const std::string &sIncomeWebhookDir,
            const std::string &sWebhooksId,
            const std::string &sPayloadContent
        );

        WebhookRequest(
            const std::string &sIncomeWebhookDir,
            const std::string &sFilename
        );

        std::string getId();

        bool saveToFile();

    private:
        std::string TAG;
        long m_nTimeRequest;
        std::string m_sIncomeWebhookDir;
        std::string m_sFilename;
        std::string m_sWebhooksId;
        std::string m_sPayloadContent;
};

class DequeWebhooks {
    public:
        DequeWebhooks(int nMaxDeque, const std::string &sIncomeWebhookDir);
        WebhookRequest popWebhook();
        void pushWebhook(
            const std::string &sWebhooksId,
            const std::string &sPayloadContent
        );
        void cleanup();

    private:
        std::string TAG;
        int m_nMaxDeque;

        std::mutex m_mtxDeque;
        std::deque<WebhookRequest> m_dequeWebhooks;
        
        std::string m_sIncomeWebhookDir;
};

#endif // DEQUE_WEBHOOKS_H
