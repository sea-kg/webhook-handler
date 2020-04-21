#include "webhook_handler_config.h"
#include <sstream>
#include <ctime>
#include <locale>
#include <date.h>
#include <iostream>
#include <sstream>
#include <conf_file_parser.h>
#include <wsjcpp_core.h>
 

Webhook::Webhook(){
    m_nScriptWaitInSec = 10;
    m_bEnabled = true;
}

// ----------------------------------------------------------------------

void Webhook::setId(const std::string &sWebhookID){
    m_sID = sWebhookID;
}

// ----------------------------------------------------------------------

std::string Webhook::id() const {
    return m_sID;
}

// ----------------------------------------------------------------------

void Webhook::setScriptPath(const std::string &sScriptPath){
    m_sScriptPath = sScriptPath;
}

// ----------------------------------------------------------------------

std::string Webhook::scriptPath() const {
    return m_sScriptPath;
}

// ----------------------------------------------------------------------

void Webhook::setScriptDir(const std::string &sScriptDir) {
    m_sScriptDir = sScriptDir;
}

// ----------------------------------------------------------------------

std::string Webhook::scriptDir() const {
    return m_sScriptDir;
}

// ----------------------------------------------------------------------

void Webhook::setEnabled(bool bEnabled){
    m_bEnabled = bEnabled;
}

// ----------------------------------------------------------------------

bool Webhook::isEnabled() const {
    return m_bEnabled;
}

// ----------------------------------------------------------------------

void Webhook::setScriptWaitInSec(int nSec){
    m_nScriptWaitInSec = nSec;
    if(m_nScriptWaitInSec < 1){
        m_nScriptWaitInSec = 10;
    }
}

// ----------------------------------------------------------------------

int Webhook::scriptWaitInSec() const {
    return m_nScriptWaitInSec;
}

// ----------------------------------------------------------------------
// WebhookHandlerConfig

WebhookHandlerConfig::WebhookHandlerConfig(const std::string &sWorkspaceDir) {
    TAG = "WebhookHandlerConfig";
    m_sWorkspaceDir = sWorkspaceDir;
    m_sConfigFile = m_sWorkspaceDir + "/webhook-handler-conf.yml";
    m_pYamlConfig = new WsjcppYaml();
    m_nServerPort = 0;
    m_nMaxDeque = 0;
    m_nMaxScriptThreads = 0;
    m_nWaitSecondsBetweenRunScripts = 0;
}

// ---------------------------------------------------------------------

bool WebhookHandlerConfig::applyWebhooksConf() {
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

bool WebhookHandlerConfig::applyConfig() {
    bool bResult = true;
    WsjcppLog::info(TAG, "Loading configuration... ");

    if (!m_pYamlConfig->loadFromFile(m_sConfigFile)) {
       return false;
    }

    std::string sServerPort = m_pYamlConfig->getRoot()->getElement("server")->getElement("port")->getValue(); 
    m_nServerPort = std::atoi(sServerPort.c_str());
    if (m_nServerPort <= 10 || m_nServerPort > 65536) {
        WsjcppLog::err(TAG, m_sConfigFile + ": wrong server port (expected value od 11..65535)");
        return -1;
    }

    std::string sMaxDeque = m_pYamlConfig->getRoot()->getElement("server")->getElement("max-deque")->getValue();
    m_nMaxDeque = std::atoi(sMaxDeque.c_str());
    if (m_nMaxDeque <= 10 || m_nMaxDeque > 1000) {
        WsjcppLog::err(TAG, m_sConfigFile + ": wrong server max-deque (expected value od 10..1000)");
        return -1;
    }

    std::string sMaxScriptThreads = m_pYamlConfig->getRoot()->getElement("server")->getElement("max-script-threads")->getValue();
    m_nMaxScriptThreads = std::atoi(sMaxScriptThreads.c_str());
    if (m_nMaxScriptThreads < 1 || m_nMaxScriptThreads > 100) {
        WsjcppLog::err(TAG, m_sConfigFile + ": wrong server max-script-threads (expected value od 1..100)");
        return -1;
    }

    std::string sWaitSecondsBetweenRunScripts = m_pYamlConfig->getRoot()->getElement("server")->getElement("wait-seconds-between-run-scripts")->getValue();
    m_nWaitSecondsBetweenRunScripts = std::atoi(sWaitSecondsBetweenRunScripts.c_str());
    if (m_nWaitSecondsBetweenRunScripts < 1 || m_nWaitSecondsBetweenRunScripts > 100) {
        WsjcppLog::err(TAG, m_sConfigFile + ": wrong server wait-seconds-between-run-scripts (expected value od 1..100)");
        return -1;
    }

    // apply the server config
    if (!this->applyWebhooksConf()) {
        return false;
    }

    return bResult;
}

// ---------------------------------------------------------------------

int WebhookHandlerConfig::getServerPort() {
    return m_nServerPort;
}

// ---------------------------------------------------------------------

int WebhookHandlerConfig::getMaxDeque() {
    return m_nMaxDeque;
}

// ---------------------------------------------------------------------

int WebhookHandlerConfig::getMaxScriptThreads() {
    return m_nMaxScriptThreads;
}

// ---------------------------------------------------------------------

int WebhookHandlerConfig::getWaitSecondsBetweenRunScripts() {
    return m_nWaitSecondsBetweenRunScripts;
}

// ---------------------------------------------------------------------

const std::vector<Webhook> &WebhookHandlerConfig::webhooksConf() {
    return m_vWebhooksConf;
}
