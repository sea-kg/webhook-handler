#include "config.h"
#include <sstream>
#include <ctime>
#include <locale>
#include <date.h>
#include <iostream>
#include <sstream>
#include <conf_file_parser.h>
#include <wsjcpp_core.h>
 
Config::Config(const std::string &sWorkspaceDir) {
    TAG = "Config";
    m_nHttpPort = 8080;
    m_sWorkspaceDir = sWorkspaceDir;
    m_nThreadsForScripts = 1;
    m_nMaxDequeWebhooks = 100;
}

// ---------------------------------------------------------------------

bool Config::applyServerConf() {
    std::string sConfigFile = m_sWorkspaceDir + "/conf.d/server.conf";
    WsjcppLog::info(TAG, "Reading config: " + sConfigFile);

    if (!WsjcppCore::fileExists(sConfigFile)) {
        WsjcppLog::err(TAG, "File " + sConfigFile + " does not exists ");
        return false;
    }

    ConfFileParser serverConf = ConfFileParser(sConfigFile);
    if (!serverConf.parseConfig()) {
        WsjcppLog::err(TAG, "Could not parse " + sConfigFile);
        return false;
    }
    
    m_nHttpPort = serverConf.getIntValueFromConfig("http_port", m_nHttpPort);
    if (m_nHttpPort <= 10 || m_nHttpPort > 65536) {
        WsjcppLog::err(TAG, sConfigFile + ": wrong http_port (expected value od 11..65535)");
        return false;
    }
    WsjcppLog::info(TAG, "http_port: " + std::to_string(m_nHttpPort));

    m_nThreadsForScripts = serverConf.getIntValueFromConfig("threads_for_scripts", m_nThreadsForScripts);
    if (m_nThreadsForScripts <= 0 || m_nThreadsForScripts > 10) {
        WsjcppLog::err(TAG, sConfigFile + ": wrong threads_for_scripts");
        return false;
    }

    m_nMaxDequeWebhooks = serverConf.getIntValueFromConfig("max_deque_webhooks", m_nMaxDequeWebhooks);
    WsjcppLog::info(TAG, "max_deque_webhooks = " + std::to_string(m_nMaxDequeWebhooks));


    m_nSleepBetweenRunScriptsInSec = serverConf.getIntValueFromConfig("sleep_between_run_scripts_in_sec", 1);
    WsjcppLog::info(TAG, "sleep_between_run_scripts_in_sec = " + std::to_string(m_nSleepBetweenRunScriptsInSec));

    return true;
}

// ---------------------------------------------------------------------

bool Config::applyWebhooksConf() {
    std::string sConfDir = m_sWorkspaceDir + "/conf.d/";
    if (!WsjcppCore::dirExists(sConfDir)) {
        WsjcppLog::err(TAG, "Directory " + sConfDir + " not exists");
        return false;
    }
    WsjcppLog::info(TAG, "Search webhook.conf");

    std::vector<std::string> vListOfWebhooks = WsjcppCore::listOfDirs(sConfDir);
    if (vListOfWebhooks.size() == 0) {
        WsjcppLog::err(TAG, "Folders with webhooks does not found in " + sConfDir);
        return false;
    }

    for (int i = 0; i < vListOfWebhooks.size(); i++) {
        std::string sFolder = vListOfWebhooks[i];
        std::string sWebhookScriptDir = sConfDir + sFolder + "/";
        std::string sWebhookConfPath =  sWebhookScriptDir + "/webhook.conf";
        WsjcppLog::info(TAG, "Reading " + sWebhookConfPath);
        if (!WsjcppCore::fileExists(sWebhookConfPath)) {
            WsjcppLog::err(TAG, "File " + sWebhookConfPath + " not exists");
            return false;
        }
        ConfFileParser serviceConf = ConfFileParser(sWebhookConfPath);
        if (!serviceConf.parseConfig()) {
            WsjcppLog::err(TAG, "Could not parse " + sWebhookConfPath);
            return false;
        }

        std::string sWebhookId = serviceConf.getStringValueFromConfig("id", "");
        WsjcppLog::info(TAG, "id = " + sWebhookId);

        bool bWebhookEnable 
            = serviceConf.getBoolValueFromConfig("enabled", false);
        WsjcppLog::info(TAG, "enabled = " + std::string(bWebhookEnable ? "yes" : "no"));

        std::string sScriptPath = serviceConf.getStringValueFromConfig("script_path", "");
        WsjcppLog::info(TAG, "script_path = " + sScriptPath);
        
        // Log::info(TAG, "sWebhookScriptDir: " + sWebhookScriptDir);
        if (!WsjcppCore::fileExists(sWebhookScriptDir + sScriptPath)) {
            WsjcppLog::err(TAG, "File " + sWebhookScriptDir + sScriptPath + " did not exists");
            return false;
        }

        int nScritpWait = serviceConf.getIntValueFromConfig("script_wait_in_sec", 60);
        WsjcppLog::info(TAG, "script_wait_in_sec = " + std::to_string(nScritpWait));

        if (nScritpWait < 5) {
            WsjcppLog::err(TAG, "Could not parse script_wait_in_sec - must be more than 4 sec ");
            return false;
        }

        if (!bWebhookEnable) {
            WsjcppLog::warn(TAG, "Webhook " + sFolder + " - disabled ");
            continue;
        }
        
        for (unsigned int i = 0; i < m_vWebhooksConf.size(); i++) {
            if (m_vWebhooksConf[i].id() == sWebhookId) {
                WsjcppLog::err(TAG, "Already registered webhook " + sWebhookId);
                return false;
            }
        }

        // default values of service config
        Webhook _webhookConf;
        _webhookConf.setId(sWebhookId);
        _webhookConf.setScriptPath(sScriptPath);
        _webhookConf.setScriptDir(sWebhookScriptDir);
        _webhookConf.setEnabled(bWebhookEnable);
        _webhookConf.setScriptWaitInSec(nScritpWait);
        m_vWebhooksConf.push_back(_webhookConf);

        WsjcppLog::ok(TAG, "Registered webhook " + sFolder + " -> /wh/" + sWebhookId);
    }

    if (m_vWebhooksConf.size() == 0) {
        WsjcppLog::err(TAG, "No one defined webhooks in " + sConfDir);
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------

bool Config::applyConfig() {
    bool bResult = true;
    WsjcppLog::info(TAG, "Loading configuration... ");
    
    // apply the server config
    if (!this->applyServerConf()) {
        return false;
    }

    // apply the server config
    if (!this->applyWebhooksConf()) {
        return false;
    }

    return bResult;
}

// ---------------------------------------------------------------------

const std::vector<Webhook> &Config::webhooksConf() {
    return m_vWebhooksConf;
}

// ---------------------------------------------------------------------

int Config::httpPort() {
    return m_nHttpPort;
}

// ---------------------------------------------------------------------

int Config::threadsForScripts() {
    return m_nThreadsForScripts;
}

// ---------------------------------------------------------------------

int Config::maxDequeWebhooks() {
    return m_nMaxDequeWebhooks;
}

// ---------------------------------------------------------------------

int Config::sleepBetweenRunScriptsInSec() {
    return m_nSleepBetweenRunScriptsInSec;
}

