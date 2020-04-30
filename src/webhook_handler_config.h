#ifndef WEBHOOK_HANDLER_CONFIG
#define WEBHOOK_HANDLER_CONFIG

#include <string>
#include <vector>
#include <wsjcpp_yaml.h>

class Webhook {
    public:
        Webhook();

        void setWebhookUrlPath(const std::string &sWebhookUrlPath);
        std::string getWebhookUrlPath() const;

        void setWorkDir(const std::string &sWorkDir);
        std::string getWorkDir() const;

        void setCommands(const std::vector<std::string> &sScriptPath);
        const std::vector<std::string> &getCommands() const;

        void setTimeoutCommand(int nSec);
        int getTimeoutCommand() const;

    private:
        int m_nScriptWaitInSec;
        std::string m_sID;
        std::string m_sWebhookUrlPath;
        std::string m_sWorkDir;
        std::string m_sUser;
        std::vector<std::string> m_vCommands;
};

// ---------------------------------------------------------------------

class WebhookHandlerConfig {
    public:
        WebhookHandlerConfig(const std::string &sWorkspaceDir);
        bool applyConfig();
        int getServerPort();
        int getMaxDeque();
        int getMaxScriptThreads();
        int getWaitSecondsBetweenRunScripts();

        const std::vector<Webhook> &webhooksConf();

    private:
        
        std::string TAG;
        
        std::string m_sWorkspaceDir;
        std::string m_sConfigFile;
        WsjcppYaml *m_pYamlConfig;
        int m_nServerPort;
        int m_nMaxDeque;
        int m_nMaxScriptThreads;
        int m_nWaitSecondsBetweenRunScripts;
        std::vector<Webhook> m_vWebhooksConf;
};

#endif // WEBHOOK_HANDLER_CONFIG
