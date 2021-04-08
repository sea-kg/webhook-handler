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
#include <iomanip>
#include <json.hpp>

// ----------------------------------------------------------------------
// WebhookRequest

WebhookRequest::WebhookRequest() {
    // empty
}

WebhookRequest::WebhookRequest(
    const std::string &sIncomeWebhookDir,
    const std::string &sWebhooksId,
    const std::string &sPayloadContent
) {
    m_sIncomeWebhookDir = sIncomeWebhookDir;
    m_sWebhooksId = sWebhooksId;
    m_sPayloadContent = sPayloadContent;
    m_nTimeRequest = WsjcppCore::getCurrentTimeInMilliseconds();
    m_sFilename = "webhook_dt" + std::to_string(m_nTimeRequest) + ".req";
    TAG = "WebhookRequest-" + m_sFilename;
}

WebhookRequest::WebhookRequest(
    const std::string &sIncomeWebhookDir,
    const std::string &sFilename
) {
    m_sIncomeWebhookDir = sIncomeWebhookDir;
    m_sFilename = sFilename;
    TAG = "WebhookRequest-" + m_sFilename;

    // TODO load data from filesystem
}

std::string WebhookRequest::getId() {
    return m_sWebhooksId;
}

bool WebhookRequest::saveToFile() {
    // keep the data to filesystem
    nlohmann::json jsonWebhookInfo;
    jsonWebhookInfo["id"] = m_sWebhooksId;
    jsonWebhookInfo["datetime"] = WsjcppCore::formatTimeUTC(m_nTimeRequest/1000);
    jsonWebhookInfo["timestemp_ms"] = m_nTimeRequest;
    jsonWebhookInfo["payload"] = m_sPayloadContent;
    std::string sFilenameReq = m_sIncomeWebhookDir + "/" + m_sFilename;
    if (WsjcppCore::fileExists(sFilenameReq)) {
        WsjcppLog::err(TAG, "File already exists");
        return false;
    }
    std::ofstream fileOutput(sFilenameReq);
    fileOutput << std::setw(4) << jsonWebhookInfo << std::endl;
    return true;
}

std::string WebhookRequest::getFilename() {
    return m_sFilename;
}

// ----------------------------------------------------------------------
// DequeWebhooks

DequeWebhooks::DequeWebhooks(int nMaxDeque, const std::string &sIncomeWebhookDir) {
    m_nMaxDeque = nMaxDeque;
    m_sIncomeWebhookDir = sIncomeWebhookDir;
    m_sFilenameProcessing = m_sIncomeWebhookDir + "/_processing_webhook";
}

WebhookRequest DequeWebhooks::popWebhook() {
    std::lock_guard<std::mutex> guard(this->m_mtxDeque);
    WebhookRequest req;
    int nSize = m_dequeWebhooks.size();
    if (nSize > 0) {
        req = m_dequeWebhooks.back();
        m_dequeWebhooks.pop_back();
        std::string sFilenameReq = m_sIncomeWebhookDir + "/" + req.getFilename();
        if (!WsjcppCore::fileExists(sFilenameReq)) {
            WsjcppLog::throw_err(TAG, "File not found '" + sFilenameReq + "'");
        }
        if (rename(sFilenameReq.c_str(), m_sFilenameProcessing.c_str()) != 0) {
            WsjcppLog::throw_err(TAG, "Error renaming '" + sFilenameReq + "' -> '" + m_sFilenameProcessing + "'");
        } else {
            WsjcppLog::info(TAG, "File renamed successfully '" + sFilenameReq + "' -> '" + m_sFilenameProcessing + "'");
        }
        // TODO move webhook to processing
    }
    return req;
}

// ----------------------------------------------------------------------

void DequeWebhooks::pushWebhook(
    const std::string &sWebhooksId,
    const std::string &sPayloadContent
) {
    std::lock_guard<std::mutex> guard(this->m_mtxDeque);

    WebhookRequest req(
        m_sIncomeWebhookDir,
        sWebhooksId,
        sPayloadContent
    );

    if (!req.saveToFile()) {
        return;
    }

    if (m_dequeWebhooks.size() > m_nMaxDeque) {
        WsjcppLog::warn(TAG, " deque more than " + std::to_string(m_dequeWebhooks.size()));
    }
    m_dequeWebhooks.push_front(req);
}

// ----------------------------------------------------------------------

void DequeWebhooks::cleanup(){
    std::lock_guard<std::mutex> guard(this->m_mtxDeque);
    while (m_dequeWebhooks.size() > 0) {
        m_dequeWebhooks.pop_back();
    }
}

// ----------------------------------------------------------------------

void DequeWebhooks::removeProcessingWebhook() {
    // TODO remove processing webhook
}

// ----------------------------------------------------------------------
