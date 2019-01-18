#ifndef WEBHOOK_H
#define WEBHOOK_H

#include <string>

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

#endif // WEBHOOK_H
