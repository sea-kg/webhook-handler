#ifndef HTTP_HANDLER_API_V1_H
#define HTTP_HANDLER_API_V1_H

#include <light_http_handler_base.h>
#include <config.h>
#include <json.hpp>
#include <deque_webhooks.h>

class HttpHandlerWebhooks : LightHttpHandlerBase {
    public:
        HttpHandlerWebhooks(Config *pConfig, DequeWebhooks *pDequeWebhooks);
        virtual bool canHandle(const std::string &sWorkerId, LightHttpRequest *pRequest);
        virtual bool handle(const std::string &sWorkerId, LightHttpRequest *pRequest);

    private:
        std::string TAG;

        Config *m_pConfig;
        DequeWebhooks *m_pDequeWebhooks;
        std::map<std::string, std::string> m_mapWebhooksPaths;
        std::vector<Webhook> m_vWebhooks;
        nlohmann::json m_jsonGame;
        nlohmann::json m_jsonTeams; // prepare data for list of teams
        nlohmann::json m_jsonServices; // prepare data for list of teams

};

#endif // HTTP_HANDLER_API_V1_H
