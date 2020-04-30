#include "webhook_handler_config.h"
#include <sstream>
#include <ctime>
#include <locale>
#include <date.h>
#include <iostream>
#include <sstream>
#include <wsjcpp_core.h>

Webhook::Webhook(){
    m_nScriptWaitInSec = 10;
}

// ----------------------------------------------------------------------

void Webhook::setWebhookUrlPath(const std::string &sWebhookUrlPath) {
    m_sWebhookUrlPath = sWebhookUrlPath;
}

// ----------------------------------------------------------------------

std::string Webhook::getWebhookUrlPath() const {
    return m_sWebhookUrlPath;
}

// ----------------------------------------------------------------------

void Webhook::setWorkDir(const std::string &sWorkDir) {
    m_sWorkDir = sWorkDir;
}

// ----------------------------------------------------------------------

std::string Webhook::getWorkDir() const {
    return m_sWorkDir;
}

// ----------------------------------------------------------------------

void Webhook::setCommands(const std::vector<std::string> &vCommands) {
    for (int i = 0; i < vCommands.size(); i++) {
        m_vCommands.push_back(vCommands[i]);
    }
    
}

// ----------------------------------------------------------------------

const std::vector<std::string> &Webhook::getCommands() const {
    return m_vCommands;
}

// ----------------------------------------------------------------------

void Webhook::setTimeoutCommand(int nSec){
    m_nScriptWaitInSec = nSec;
    if(m_nScriptWaitInSec < 1){
        m_nScriptWaitInSec = 10;
    }
}

// ----------------------------------------------------------------------

int Webhook::getTimeoutCommand() const {
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

    WsjcppYamlItem *pHandlers = m_pYamlConfig->getRoot()->getElement("webhook-handlers");
    std::vector<std::string> vKeys = pHandlers->getKeys();
    for (int i = 0; i < vKeys.size(); i++) {
        std::string sName = vKeys[i];
        WsjcppYamlItem *pWebhookConf = pHandlers->getElement(sName);
        std::vector<std::string> vWebhookKeys = pWebhookConf->getKeys();

        bool bEnabled = true;
        Webhook _webhookConf;
        
        for (int i = 0; i < vWebhookKeys.size(); i++) {
            std::string sWebhookParamName = vWebhookKeys[i];
            std::string sLogFormat = ", in " + pWebhookConf->getElement(sWebhookParamName)->getForLogFormat();

            if (sWebhookParamName == "enabled") {
                std::string sEnabled = pWebhookConf->getElement("enabled")->getValue();
                if (sEnabled == "no" || sEnabled == "false") {
                    bEnabled = false;
                    break;
                }
            } else if (sWebhookParamName == "work-dir") {
                std::string sWorkDir = pWebhookConf->getElement(sWebhookParamName)->getValue();
                sWorkDir = WsjcppCore::doNormalizePath(m_sWorkspaceDir + "/" + sWorkDir);
                if (!WsjcppCore::dirExists(sWorkDir)) {
                    WsjcppLog::err(TAG, "Folder '" + sWorkDir + "' did not exists" + sLogFormat);
                    return -1;
                }
                _webhookConf.setWorkDir(sWorkDir);
            } else if (sWebhookParamName == "webhook-url-path") {
                std::string sWebhookUrlPath = pWebhookConf->getElement(sWebhookParamName)->getValue();
                if (sWebhookUrlPath == "") {
                    WsjcppLog::err(TAG, "Require 'webhook-url-path' in '" + sName + "'" + sLogFormat);
                    return -1;
                }
                // TODO check format sWebhookUrlPath
                _webhookConf.setWebhookUrlPath(sWebhookUrlPath);
            } else if (sWebhookParamName == "timeout-command") {
                std::string sTimeoutCommand = pWebhookConf->getElement(sWebhookParamName)->getValue();
                int nTimeoutCommand = std::atoi(sTimeoutCommand.c_str());
                if (nTimeoutCommand < 5) {
                    WsjcppLog::err(TAG, "Wrong value for 'timeout-command' - must be more than 4 sec " + sLogFormat);
                    return false;
                }
                _webhookConf.setTimeoutCommand(nTimeoutCommand);
            } else if (sWebhookParamName == "commands") {
                WsjcppYamlItem *pCommands = pWebhookConf->getElement(sWebhookParamName);
                std::vector<std::string> vCommands;
                int nLen = pCommands->getLength();
                for (int i = 0; i < nLen; i++) {
                    vCommands.push_back(pCommands->getElement(i)->getValue());
                }
                _webhookConf.setCommands(vCommands);
            } else {
                WsjcppLog::warn(TAG, "Unknown key: '" + sWebhookParamName + "'" + sLogFormat);
            }
        }

        if (!bEnabled) {
            WsjcppLog::info(TAG, "Webhook '" + sName + "' disbaled");
            continue;
        }

        for (unsigned int i = 0; i < m_vWebhooksConf.size(); i++) {
            if (m_vWebhooksConf[i].getWebhookUrlPath() == _webhookConf.getWebhookUrlPath()) {
                WsjcppLog::err(TAG, "Already registered webhook " + _webhookConf.getWebhookUrlPath());
                return false;
            }
        }

        if (_webhookConf.getCommands().size() == 0) {
            WsjcppLog::err(TAG, "Must contain at least one command " + _webhookConf.getWebhookUrlPath());
            return false;
        }

        // TODO user
        WsjcppLog::ok(TAG, "Registered webhook: " + _webhookConf.getWebhookUrlPath());
        m_vWebhooksConf.push_back(_webhookConf);
    }

    if (m_vWebhooksConf.size() == 0) {
        WsjcppLog::err(TAG, "No configured webhooks");
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
