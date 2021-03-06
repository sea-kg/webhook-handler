#include "deque_webhooks.h"
#include <wsjcpp_core.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <sys/time.h>
#include <fstream>
#include <regex>        // regex, sregex_token_iterator
#include <stdio.h>
#include <math.h>
#include <json.hpp>

DequeWebhooks::DequeWebhooks(int nMaxDeque, const std::string &sIncomeWebhookDir) {
    m_nMaxDeque = nMaxDeque;
    m_sIncomeWebhookDir = sIncomeWebhookDir;
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

void DequeWebhooks::pushWebhookId(
    const std::string &sWebhooksId,
    const std::string &sPayloadContent
) {
    std::lock_guard<std::mutex> guard(this->m_mtxDeque);
    long nT = WsjcppCore::getCurrentTimeInMilliseconds();

    // keep the data to filesystem
    nlohmann::json jsonWebhookInfo;
    jsonWebhookInfo["id"] = sWebhooksId;
    jsonWebhookInfo["datetime"] = WsjcppCore::getCurrentTimeForLogFormat();
    jsonWebhookInfo["timestemp_ms"] = nT;
    jsonWebhookInfo["payload"] = sPayloadContent;
    std::string sFilenameReq = m_sIncomeWebhookDir + "/webhook_dt" + std::to_string(nT) + ".req";
    if (WsjcppCore::fileExists(sFilenameReq)) {
        WsjcppLog::err(TAG, "File already exists");
        return;
    }
    std::ofstream fileOutput(sFilenameReq);
    fileOutput << jsonWebhookInfo;

    if (m_dequeWebhooksId.size() > m_nMaxDeque) {
        WsjcppLog::warn(TAG, " deque more than " + std::to_string(m_dequeWebhooksId.size()));
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
