#ifndef WEBHOOK_HANDLER_CONFIG
#define WEBHOOK_HANDLER_CONFIG

#include <string>
#include <vector>
#include <wsjcpp_yaml.h>

class Webhook {
    public:
        Webhook();
        
        void setId(const std::string &sId);
        std::string id() const;

        void setScriptPath(const std::string &sScriptPath);
        std::string scriptPath() const;

        void setScriptDir(const std::string &sScriptDir);
        std::string scriptDir() const;

        void setEnabled(bool bEnabled);
        bool isEnabled() const;

        void setScriptWaitInSec(int nSec);
        int scriptWaitInSec() const;

    private:
        bool m_bEnabled;
        int m_nScriptWaitInSec;
        std::string m_sID;
        std::string m_sScriptPath;
        std::string m_sScriptDir;
        
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
        bool applyWebhooksConf();
        
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
