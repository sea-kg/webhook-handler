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
		DequeWebhooks(int nMaxDeque);
        std::string popWebhookId();
		void pushWebhookId(const std::string &sWebhooksId);
		void cleanup();

	private:
		std::string TAG;
		int m_nMaxDeque;

		std::mutex m_mtxDeque;
		std::deque<std::string> m_dequeWebhooksId;
};

#endif // DEQUE_WEBHOOKS_H