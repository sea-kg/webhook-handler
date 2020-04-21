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
        m_mapWebhooksPaths["/wh/" + wh.id()] = wh.id(); 
        m_vWebhooks.push_back(wh);
    }
}

// ----------------------------------------------------------------------

bool HttpHandlerWebhooks::canHandle(const std::string &sWorkerId, WsjcppLightWebHttpRequest *pRequest) {
    // std::string _tag = TAG + "-" + sWorkerId;
    std::string sPath = pRequest->getRequestPath();
    std::map<std::string,std::string>::iterator it;
    return m_mapWebhooksPaths.find(sPath) != m_mapWebhooksPaths.end();
}

// ----------------------------------------------------------------------

bool HttpHandlerWebhooks::handle(const std::string &sWorkerId, WsjcppLightWebHttpRequest *pRequest){
    std::string _tag = TAG + "-" + sWorkerId;
    WsjcppLightWebHttpResponse response(pRequest->getSockFd());
    std::string sPath = pRequest->getRequestPath();
    
    std::map<std::string,std::string>::iterator it;
    it = m_mapWebhooksPaths.find(sPath);
    if (it != m_mapWebhooksPaths.end()) {
        m_pDequeWebhooks->pushWebhookId(it->second);
        response.ok().sendText("OK");
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------
