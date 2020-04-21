#ifndef HTTP_HANDLER_WEBHOOKS_H
#define HTTP_HANDLER_WEBHOOKS_H

#include <webhook_handler_config.h>
#include <json.hpp>
#include <deque_webhooks.h>
#include <wsjcpp_light_web_server.h>

class HttpHandlerWebhooks : public WsjcppLightWebHttpHandlerBase {
    public:
        HttpHandlerWebhooks(WebhookHandlerConfig *pConfig, DequeWebhooks *pDequeWebhooks);
        virtual bool canHandle(const std::string &sWorkerId, WsjcppLightWebHttpRequest *pRequest);
        virtual bool handle(const std::string &sWorkerId, WsjcppLightWebHttpRequest *pRequest);

    private:
        std::string TAG;

        WebhookHandlerConfig *m_pConfig;
        DequeWebhooks *m_pDequeWebhooks;
        std::map<std::string, std::string> m_mapWebhooksPaths;
        std::vector<Webhook> m_vWebhooks;
        nlohmann::json m_jsonGame;
        nlohmann::json m_jsonTeams; // prepare data for list of teams
        nlohmann::json m_jsonServices; // prepare data for list of teams

};

#endif // HTTP_HANDLER_WEBHOOKS_H
