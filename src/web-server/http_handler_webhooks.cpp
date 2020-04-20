#include "http_handler_webhooks.h"
#include <fstream>
#include <string>
#include <regex>
#include <algorithm>
#include <time.h>

#include <logger.h>
#include <light_http_request.h>
#include <light_http_response.h>
#include <ts.h>
#include <str.h>

// ----------------------------------------------------------------------

HttpHandlerWebhooks::HttpHandlerWebhooks(Config *pConfig, DequeWebhooks *pDequeWebhooks) 
    : LightHttpHandlerBase("webhooks") {

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

bool HttpHandlerWebhooks::canHandle(const std::string &sWorkerId, LightHttpRequest *pRequest) {
    // std::string _tag = TAG + "-" + sWorkerId;
    std::string sPath = pRequest->requestPath();
    std::map<std::string,std::string>::iterator it;
    return m_mapWebhooksPaths.find(sPath) != m_mapWebhooksPaths.end();
}

// ----------------------------------------------------------------------

bool HttpHandlerWebhooks::handle(const std::string &sWorkerId, LightHttpRequest *pRequest){
    std::string _tag = TAG + "-" + sWorkerId;
    LightHttpResponse response(pRequest->sockFd());
    std::string sPath = pRequest->requestPath();
    
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
