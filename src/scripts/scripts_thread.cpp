#include "scripts_thread.h"
#include <unistd.h>

#include <dorunscript.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <wsjcpp_core.h>

// ---------------------------------------------------------------------

ScriptsThread::ScriptsThread(Config *pConfig, int nNumber, DequeWebhooks *pDequeWebhooks) {
    
    m_pConfig = pConfig;
    m_pDequeWebhooks = pDequeWebhooks;
    m_nNumber = nNumber;

    TAG = "ScriptsThread_" + std::to_string(m_nNumber);
    WsjcppLog::info(TAG, "Created thread");
}

// ----------------------------------------------------------------------
// newRequest

void* newScriptsThread(void *arg) {
    // Log::info("newRequest", "");
    ScriptsThread *pServerThread = (ScriptsThread *)arg;
    pthread_detach(pthread_self());
    pServerThread->run();
    return 0;
}

// ----------------------------------------------------------------------

void ScriptsThread::start() {
    pthread_create(&m_checkerThread, NULL, &newScriptsThread, (void *)this);
}

// ---------------------------------------------------------------------

int ScriptsThread::runScript(const std::string &sCommand) {

    // Used code from here
    // https://stackoverflow.com/questions/478898/how-to-execute-a-command-and-get-output-of-command-within-c-using-posix

    std::string sShellCommand = " " + sCommand;

    WsjcppLog::info(TAG, "Start script " + sShellCommand);

    /*DoRunChecker process(m_serviceConf.scriptDir(), m_serviceConf.scriptPath());
    process.start(m_serviceConf.scriptWaitInSec()*1000);

    if (process.isTimeout()) {
        return ScriptsThread::CHECKER_CODE_MUMBLE;
    }

    if (process.hasError()) {
        WsjcppLog::err(TAG, "Checker is shit");
        WsjcppLog::err(TAG, "Error on run script service: " + process.outputString());
        return ScriptsThread::CHECKER_CODE_SHIT;
    }

    int nExitCode = process.exitCode();
    if (nExitCode != ScriptsThread::CHECKER_CODE_UP 
        && nExitCode != ScriptsThread::CHECKER_CODE_MUMBLE
        && nExitCode != ScriptsThread::CHECKER_CODE_CORRUPT
        && nExitCode != ScriptsThread::CHECKER_CODE_DOWN) {
        Log::err(TAG, " Wrong checker exit code...\n"
            "\n" + process.outputString());
        return ScriptsThread::CHECKER_CODE_SHIT;
    }
    
    return nExitCode;
    */
   return 0;
}

// ---------------------------------------------------------------------

void ScriptsThread::run() {
    // TODO: BUG: so here can be problem with mysql connection after 7-8 hours (terminate connection on MySQL side)
    // SOLUTION migrate to PostgreSQL

    // TODO check if game ended

    WsjcppLog::info(TAG, "Starting thread...");
    /*if (QString::fromStdString(m_teamConf.ipAddress()).isEmpty()) {
        Log::err(TAG, "User IP Address is empty!!!");
        return;
    }*/

    // std::string sScriptPath = m_serviceConf.scriptPath();
    /*
    // already checked on start
    if (!Log::fileExists(sScriptPath)) {
        WsjcppLog::err(TAG, "FAIL: Script Path to checker not found '" + sScriptPath + "'");
        // TODO shit status
        return;
    }*/

    while(1) {

        std::this_thread::sleep_for(std::chrono::seconds(m_pConfig->sleepBetweenRunScriptsInSec()));

        std::string sWebhookId = m_pDequeWebhooks->popWebhookId();
        if (sWebhookId == "") {
            continue;
        }

        WsjcppLog::info(TAG, "Start execute webhook " + sWebhookId);
        int nSize = m_pConfig->webhooksConf().size();
        Webhook webhook;
        bool bFound = false;
        for (int i = 0; i < nSize; i++) {
            if (m_pConfig->webhooksConf()[i].id() == sWebhookId) {
                bFound = true;
                webhook = m_pConfig->webhooksConf()[i];
            };
        }
        if (!bFound) {
            WsjcppLog::err(TAG, "Not found webhook " + sWebhookId);
            continue;
        }

        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();

        DoRunScript process(webhook.scriptDir(), webhook.scriptPath());
        process.start(webhook.scriptWaitInSec()*1000);

        if (process.isTimeout()) {
            WsjcppLog::err(TAG, "Finished by timeout " + sWebhookId);
            WsjcppLog::err(TAG, process.outputString());
            continue;
        }

        if (process.hasError()) {
            WsjcppLog::err(TAG, "Script failed");
            WsjcppLog::err(TAG, "Error on run script: " + process.outputString());
            continue;
        }

        int nExitCode = process.exitCode();
        if (nExitCode != 0) {
            WsjcppLog::err(TAG, "Wrong script exit code " + std::to_string(nExitCode) + "...\n"
                "\n" + process.outputString());
            continue;
        } else {
            WsjcppLog::info(TAG, "Output:\n" + process.outputString());
            WsjcppLog::ok(TAG, "Script done.");

        }
        end = std::chrono::system_clock::now();

        int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
        WsjcppLog::info(TAG, "Elapsed milliseconds: " + std::to_string(elapsed_milliseconds) + "ms");
    }
}
