#include "webhook.h"

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
