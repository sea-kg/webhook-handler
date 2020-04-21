#ifndef CONFIG_H
#define CONFIG_H

#include <webhook.h>
#include <vector>

class Config {
    public:
        Config(const std::string &sWorkspaceDir);
        bool applyConfig();

        const std::vector<Webhook> &webhooksConf();

    private:
        bool applyWebhooksConf();
        
        std::string TAG;
        
        std::string m_sWorkspaceDir;
        std::vector<Webhook> m_vWebhooksConf;
};

#endif // CONFIG
