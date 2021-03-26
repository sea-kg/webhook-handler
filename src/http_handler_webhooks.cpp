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
    WsjcppLightWebHttpResponse response(pRequest->getSockFd());
    std::string sPath = pRequest->getRequestPath();
    // WsjcppLog::info(_tag, pRequest->getRequestBody());
    m_pDequeWebhooks->pushWebhook(sPath, pRequest->getRequestBody());
    response.ok().sendText("OK");
    return true;
}

// ----------------------------------------------------------------------
