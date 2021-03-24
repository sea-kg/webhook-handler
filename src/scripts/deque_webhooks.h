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

class DequeWebhooks {
    public:
        DequeWebhooks(int nMaxDeque, const std::string &sIncomeWebhookDir);
        std::string popWebhookId();
        void pushWebhookId(
            const std::string &sWebhooksId,
            const std::string &sPayloadContent
        );
        void cleanup();

    private:
        std::string TAG;
        int m_nMaxDeque;

        std::mutex m_mtxDeque;
        std::deque<std::string> m_dequeWebhooksId;
        std::string m_sIncomeWebhookDir;
};

#endif // DEQUE_WEBHOOKS_H