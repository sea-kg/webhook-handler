#include "webhook_handler_config.h"
#include <sstream>
#include <ctime>
#include <locale>
#include <date.h>
#include <iostream>
#include <sstream>
#include <wsjcpp_core.h>

// WebhookShellCommand
WebhookShellCommand::WebhookShellCommand(std::string sCommand) {
    m_sCommand = sCommand;
    this->parseCommand();
}

bool WebhookShellCommand::isValid(std::string &sError) {
    sError = "";
    for (int i = 0; i < m_vArgs.size(); i++) {
        std::string sArg = m_vArgs[i];
        if (
            sArg == ">>"
            || sArg == ">"
            || sArg == "2>"
            || sArg == "1>"
            || sArg == "&>"
        ) {
            sError = "Did not allowed use a '" + sArg + "'";
            return false;
        }
    }
    return true;
}

const std::vector<std::string> &WebhookShellCommand::getArgs() const {
    return m_vArgs;
}

void WebhookShellCommand::parseCommand() {
    std::string sToken = "";
    int nState = 0;
    for (int i = 0; i < m_sCommand.length(); i++) {
        if (nState == 0 && m_sCommand[i] == '"') {
            sToken += m_sCommand[i];
            nState = 1; // string double quote
        } else if (nState == 1 && m_sCommand[i] != '"') {
            sToken += m_sCommand[i];
        } else if (nState == 1 && m_sCommand[i] == '"') {
            sToken += m_sCommand[i];
            nState = 0; // end string double quote
        } else if (nState == 0 && m_sCommand[i] == '\'') {
            sToken += m_sCommand[i];
            nState = 2; // string single quote
        } else if (nState == 2 && m_sCommand[i] != '\'') {
            sToken += m_sCommand[i];
        } else if (nState == 2 && m_sCommand[i] == '\'') {
            sToken += m_sCommand[i];
            nState = 0; // end string double quote
        } else if (nState == 0 && m_sCommand[i] != ' ') {
            sToken += m_sCommand[i];
        } else if (nState == 0 && m_sCommand[i] == ' ') {
            sToken = WsjcppCore::trim(sToken);
            if (sToken != "") {
                m_vArgs.push_back(sToken);
                sToken = "";
            }
        }
    }
    sToken = WsjcppCore::trim(sToken);
    if (sToken != "") {
        m_vArgs.push_back(sToken);
    }
}

// ----------------------------------------------------------------------
// Webhook

Webhook::Webhook(){
    m_nScriptWaitInSec = 10;
}

void Webhook::setWebhookUrlPath(const std::string &sWebhookUrlPath) {
    m_sWebhookUrlPath = sWebhookUrlPath;
}

std::string Webhook::getWebhookUrlPath() const {
    return m_sWebhookUrlPath;
}

void Webhook::setWorkDir(const std::string &sWorkDir) {
    m_sWorkDir = sWorkDir;
}

std::string Webhook::getWorkDir() const {
    return m_sWorkDir;
}

void Webhook::setCommands(const std::vector<WebhookShellCommand> &vCommands) {
    for (int i = 0; i < vCommands.size(); i++) {
        m_vCommands.push_back(vCommands[i]);
    }
}

const std::vector<WebhookShellCommand> &Webhook::getCommands() const {
    return m_vCommands;
}

void Webhook::setTimeoutCommand(int nSec){
    m_nScriptWaitInSec = nSec;
    if(m_nScriptWaitInSec < 1){
        m_nScriptWaitInSec = 10;
    }
}

int Webhook::getTimeoutCommand() const {
    return m_nScriptWaitInSec;
}

void Webhook::setUser(const std::string &sUser) {
    m_sUser = sUser;
}

std::string Webhook::getUser() {
    return m_sUser;
}

// ----------------------------------------------------------------------
// WebhookHandlerConfig

WebhookHandlerConfig::WebhookHandlerConfig(const std::string &sWorkspaceDir) {
    TAG = "WebhookHandlerConfig";
    m_sWorkspaceDir = sWorkspaceDir;
    m_sConfigFile = m_sWorkspaceDir + "/webhook-handler-conf.yml";
    m_bAllowedStatusPage = false;
    m_pYamlConfig = new WsjcppYaml();
    m_nServerPort = 8002; // default port
    m_nMaxDeque = 100; // default max-len deque
    m_nMaxScriptThreads = 1;
    m_nWaitSecondsBetweenRunScripts = 60;
    m_sLogDir = "/var/log/webhook-handler/";
    m_sStatusPageUrlPath = "/wh/status";
    m_sIncomeWebhooksDir = "/var/lib/webhook-handler/income-webhooks/";
}

WebhookHandlerConfig::~WebhookHandlerConfig() {
    delete m_pYamlConfig;
}

// ---------------------------------------------------------------------

bool WebhookHandlerConfig::applyConfig() {
    bool bResult = true;
    WsjcppLog::info(TAG, "Loading configuration... ");
    std::string sError;
    if (!m_pYamlConfig->loadFromFile(m_sConfigFile, sError)) {
        WsjcppLog::err(TAG, sError);
        return false;
    }

    if (!this->applyServerConfig()) {
        return false;
    }

    WsjcppYamlNode *pHandlers = m_pYamlConfig->getRoot()->getElement("webhook-handlers");
    std::vector<std::string> vKeys = pHandlers->getKeys();
    for (int i = 0; i < vKeys.size(); i++) {
        std::string sName = vKeys[i];
        WsjcppLog::info(TAG, " **** loading '" + sName + "'");
        WsjcppYamlNode *pWebhookConf = pHandlers->getElement(sName);
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
            } else if (sWebhookParamName == "workdir") {
                std::string sWorkDir = pWebhookConf->getElement(sWebhookParamName)->getValue();
                sWorkDir = WsjcppCore::doNormalizePath(m_sWorkspaceDir + "/" + sWorkDir);
                if (!WsjcppCore::dirExists(sWorkDir)) {
                    WsjcppLog::err(TAG, "Folder '" + sWorkDir + "' did not exists" + sLogFormat);
                    return false;
                }
                _webhookConf.setWorkDir(sWorkDir);
            } else if (sWebhookParamName == "webhook-url-path") {
                std::string sWebhookUrlPath = pWebhookConf->getElement(sWebhookParamName)->getValue();
                if (sWebhookUrlPath == "") {
                    WsjcppLog::err(TAG, "Require 'webhook-url-path' in '" + sName + "'" + sLogFormat);
                    return false;
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
                WsjcppYamlNode *pCommands = pWebhookConf->getElement(sWebhookParamName);
                std::vector<WebhookShellCommand> vCommands;
                int nLen = pCommands->getLength();
                for (int i = 0; i < nLen; i++) {
                    std::string sCommand = pCommands->getElement(i)->getValue();
                    WsjcppLog::info(TAG, "Command = " + sCommand);
                    WebhookShellCommand shellCmd(sCommand);
                    std::string sError;
                    if (!shellCmd.isValid(sError)) {
                        WsjcppLog::err(TAG, "Wrong value for 'command' '" + sCommand + "' \n   " + sError + "\n   " + sLogFormat);
                    }
                    vCommands.push_back(shellCmd);
                }
                _webhookConf.setCommands(vCommands);
            } else if (sWebhookParamName == "user") {
                std::string sUser = pWebhookConf->getElement(sWebhookParamName)->getValue();
                // TODO check user in system
                _webhookConf.setUser(sUser);
            } else {
                WsjcppLog::warn(TAG, "Unknown key: '" + sWebhookParamName + "'" + sLogFormat);
            }
        }

        if (!bEnabled) {
            WsjcppLog::info(TAG, "Webhook '" + sName + "' with path '" + _webhookConf.getWebhookUrlPath() + "' disabled");
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

int WebhookHandlerConfig::getServerPort() {
    return m_nServerPort;
}

int WebhookHandlerConfig::getMaxDeque() {
    return m_nMaxDeque;
}

int WebhookHandlerConfig::getMaxScriptThreads() {
    return m_nMaxScriptThreads;
}

int WebhookHandlerConfig::getWaitSecondsBetweenRunScripts() {
    return m_nWaitSecondsBetweenRunScripts;
}

std::string WebhookHandlerConfig::getIncomeWebhookDir() {
    return m_sIncomeWebhooksDir;
}

const std::vector<Webhook> &WebhookHandlerConfig::webhooksConf() {
    return m_vWebhooksConf;
}

bool WebhookHandlerConfig::isAllowedStatusPage() {
    return m_bAllowedStatusPage;
}

bool WebhookHandlerConfig::applyServerConfig() {
    WsjcppYamlCursor cur = m_pYamlConfig->getCursor();

    if (!cur.hasKey("server")) {
        return false;
    }
    
    WsjcppYamlCursor curServer = cur["server"];

    std::vector<std::string> vKeys = curServer.keys();
    for (int i = 0; i < vKeys.size(); i++) {
        std::string sKey = vKeys[i];
        std::string sLogFormat = ", in " + curServer[sKey].node()->getForLogFormat();

        if (sKey == "port") {
            m_nServerPort = curServer[sKey].valInt();
            if (m_nServerPort <= 10 || m_nServerPort > 65536) {
                WsjcppLog::err(TAG, m_sConfigFile + ": wrong server port (expected value od 11..65535)");
                return false;
            }
        } else if (sKey == "max-deque") {
            m_nMaxDeque = curServer[sKey].valInt();
            if (m_nMaxDeque <= 10 || m_nMaxDeque > 1000) {
                WsjcppLog::err(TAG, m_sConfigFile + ": wrong server max-deque (expected value od 10..1000)");
                return false;
            }
        } else if (sKey == "max-script-threads") {
            m_nMaxScriptThreads = curServer[sKey].valInt();
            if (m_nMaxScriptThreads < 1 || m_nMaxScriptThreads > 100) {
                WsjcppLog::err(TAG, m_sConfigFile + ": wrong server max-script-threads (expected value od 1..100)");
                return false;
            }
        } else if (sKey == "wait-seconds-between-run-scripts") {
            m_nWaitSecondsBetweenRunScripts = curServer[sKey].valInt();
            if (m_nWaitSecondsBetweenRunScripts < 1 || m_nWaitSecondsBetweenRunScripts > 100) {
                WsjcppLog::err(TAG, m_sConfigFile + ": wrong server wait-seconds-between-run-scripts (expected value od 1..100)");
                return false;
            }
        } else if (sKey == "allow-status-page") {
            m_bAllowedStatusPage = curServer[sKey].valBool();
        } else if (sKey == "status-page-url-path") {
            m_sStatusPageUrlPath = curServer[sKey].valStr();
        } else if (sKey == "log-dir") {
            m_sLogDir = curServer[sKey].valStr();
        } else if (sKey == "income-webhooks-dir") {
            m_sIncomeWebhooksDir = curServer[sKey].valStr();
            if (m_sIncomeWebhooksDir.size() > 0 && m_sIncomeWebhooksDir[0] != '/') {
                m_sIncomeWebhooksDir = WsjcppCore::doNormalizePath(m_sWorkspaceDir + "/" + m_sIncomeWebhooksDir);
            }
        } else {
            WsjcppLog::warn(TAG, "Unknown key: '" + sKey + "'" + sLogFormat);
        }
    }
    return true;
}
