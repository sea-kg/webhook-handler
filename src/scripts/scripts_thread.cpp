#include "scripts_thread.h"
#include <unistd.h>

#include <do_run_command.h>
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

ScriptsThread::ScriptsThread(
    WebhookHandlerConfig *pConfig, 
    int nWaitSecondsBetweenRunScripts,
    int nNumber,
    DequeWebhooks *pDequeWebhooks
) {
    TAG = "ScriptsThread_" + std::to_string(m_nNumber);
    
    m_pConfig = pConfig;
    m_nWaitSecondsBetweenRunScripts = nWaitSecondsBetweenRunScripts;
    m_pDequeWebhooks = pDequeWebhooks;
    m_nNumber = nNumber;

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

void ScriptsThread::run() {
    WsjcppLog::info(TAG, "Starting thread...");
    while(1) {

        std::this_thread::sleep_for(std::chrono::seconds(m_nWaitSecondsBetweenRunScripts));
        
        WebhookRequest req = m_pDequeWebhooks->popWebhook();
        if (req.getId() == "") {
            m_pDequeWebhooks->removeProcessingWebhook();
            continue;
        }

        WsjcppLog::info(TAG, "Start handling webhook " + req.getId());
        int nSize = m_pConfig->webhooksConf().size();
        Webhook webhook;
        bool bFound = false;
        for (int i = 0; i < nSize; i++) {
            if (m_pConfig->webhooksConf()[i].getWebhookUrlPath() == req.getId()) {
                bFound = true;
                webhook = m_pConfig->webhooksConf()[i];
            };
        }
        if (!bFound) {
            WsjcppLog::err(TAG, "Not found webhook " + req.getId());
            m_pDequeWebhooks->removeProcessingWebhook();
            continue;
        }

        std::chrono::time_point<std::chrono::system_clock> start_all, end_all, start, end;
        start_all = std::chrono::system_clock::now();
        std::vector<WebhookShellCommand> vCommands = webhook.getCommands();
        std::string sWorkDir = webhook.getWorkDir();
        int nUserId = webhook.getUserId();
        int nGroupId = webhook.getUserId();
        for (int i = 0; i < vCommands.size(); i++) {
            start = std::chrono::system_clock::now();

            DoRunCommand process(sWorkDir, vCommands[i].getArgs(), nUserId, nGroupId);
            process.start(webhook.getTimeoutCommand()*1000);

            if (process.isTimeout()) {
                WsjcppLog::err(TAG, "Comamnd finished by timeout " + req.getId());
                WsjcppLog::err(TAG, process.outputString());
                break;
            }

            if (process.hasError()) {
                WsjcppLog::err(TAG, "Command failed");
                WsjcppLog::err(TAG, "Error on run command: " + process.outputString());
                break;
            }

            int nExitCode = process.exitCode();
            if (nExitCode != 0) {
                WsjcppLog::err(TAG, "Wrong command exit code " + std::to_string(nExitCode) + "...\n"
                    "\nOutput:" + process.outputString());
                WsjcppLog::info(TAG, "Wait next...");
                break;
            } else {
                WsjcppLog::info(TAG, "Output:\n" + process.outputString());
                WsjcppLog::ok(TAG, "Command done.");
            }
            // WsjcppLog
            end = std::chrono::system_clock::now();
            int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
            WsjcppLog::info(TAG, "Elapsed milliseconds: " + std::to_string(elapsed_milliseconds) + "ms");
        }
        end_all = std::chrono::system_clock::now();
        int elapsed_milliseconds2 = std::chrono::duration_cast<std::chrono::milliseconds>(end_all - start_all).count();
        WsjcppLog::info(TAG, "Elapsed for all milliseconds: " + std::to_string(elapsed_milliseconds2) + "ms");
        m_pDequeWebhooks->removeProcessingWebhook();
    }
}
