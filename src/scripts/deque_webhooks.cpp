#include "deque_webhooks.h"
#include <logger.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <sys/time.h>
#include <fstream>
#include <regex>        // regex, sregex_token_iterator
#include <stdio.h>
#include <math.h>
#include <fs.h>

DequeWebhooks::DequeWebhooks(int nMaxDeque) {
	m_nMaxDeque = nMaxDeque;
}

// ----------------------------------------------------------------------

std::string DequeWebhooks::popWebhookId() {
	std::lock_guard<std::mutex> guard(this->m_mtxDeque);
	std::string sWebhookId = "";
	int nSize = m_dequeWebhooksId.size();
	if (nSize > 0) {
		sWebhookId = m_dequeWebhooksId.back();
		m_dequeWebhooksId.pop_back();
	}
	return sWebhookId;
}

// ----------------------------------------------------------------------

void DequeWebhooks::pushWebhookId(const std::string &sWebhooksId) {
	std::lock_guard<std::mutex> guard(this->m_mtxDeque);
	if (m_dequeWebhooksId.size() > m_nMaxDeque) {
		Log::warn(TAG, " deque more than " + std::to_string(m_dequeWebhooksId.size()));
	}
	m_dequeWebhooksId.push_front(sWebhooksId);
}

// ----------------------------------------------------------------------

void DequeWebhooks::cleanup(){
	std::lock_guard<std::mutex> guard(this->m_mtxDeque);
	while (m_dequeWebhooksId.size() > 0) {
		m_dequeWebhooksId.pop_back();
	}
}

// ----------------------------------------------------------------------