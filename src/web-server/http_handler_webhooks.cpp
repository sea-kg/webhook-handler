#include "http_handler_webhooks.h"
#include <fstream>
#include <string>
#include <regex>
#include <algorithm>
#include <time.h>
#include <wsjcpp_core.h>

// ----------------------------------------------------------------------

HttpHandlerWebhooks::HttpHandlerWebhooks(WebhookHandlerConfig *pConfig, DequeWebhooks *pDequeWebhooks) 
    : WsjcppLightWebHttpHandlerBase("webhooks") {

    m_pConfig = pConfig;    
    m_pDequeWebhooks = pDequeWebhooks;

    TAG = "HttpHandlerWebhooks";
    int nCount = m_pConfig->webhooksConf().size();
    for (int i = 0; i < nCount; i++) {
        Webhook wh = m_pConfig->webhooksConf()[i];
        m_mapWebhooksPaths[wh.getWebhookUrlPath()] = wh; 
        m_vWebhooks.push_back(wh);
    }
}

// ----------------------------------------------------------------------

bool HttpHandlerWebhooks::canHandle(const std::string &sWorkerId, WsjcppLightWebHttpRequest *pRequest) {
    // std::string _tag = TAG + "-" + sWorkerId;
    std::string sPath = pRequest->getRequestPath();
    std::map<std::string,Webhook>::iterator it;
    return m_mapWebhooksPaths.find(sPath) != m_mapWebhooksPaths.end();
}

// ----------------------------------------------------------------------

bool HttpHandlerWebhooks::handle(const std::string &sWorkerId, WsjcppLightWebHttpRequest *pRequest){
    std::string _tag = TAG + "-" + sWorkerId;
    long nT = WsjcppCore::getCurrentTimeInSeconds();

    std::string sFilenamePayload = "webhook_dt" + std::to_string(nT) + ".payload";
    std::string sFilenameRequest = "webhook_dt" + std::to_string(nT) + ".req";

    // TODO save request body to file "webhook_dt" + datetime + ".req"
    // WsjcppLog::info(_tag, pRequest->getRequestBody());
    WsjcppLightWebHttpResponse response(pRequest->getSockFd());
    std::string sPath = pRequest->getRequestPath();
    m_pDequeWebhooks->pushWebhookId(sPath);
    response.ok().sendText("OK");
    return true;
}

// ----------------------------------------------------------------------
