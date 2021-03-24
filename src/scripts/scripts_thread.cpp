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
    WsjcppLog::info(TAG, "Starting thread...");
    while(1) {

        std::this_thread::sleep_for(std::chrono::seconds(m_nWaitSecondsBetweenRunScripts));

        std::string sWebhookId = m_pDequeWebhooks->popWebhookId();
        if (sWebhookId == "") {
            continue;
        }

        WsjcppLog::info(TAG, "Start handling webhook " + sWebhookId);
        int nSize = m_pConfig->webhooksConf().size();
        Webhook webhook;
        bool bFound = false;
        for (int i = 0; i < nSize; i++) {
            if (m_pConfig->webhooksConf()[i].getWebhookUrlPath() == sWebhookId) {
                bFound = true;
                webhook = m_pConfig->webhooksConf()[i];
            };
        }
        if (!bFound) {
            WsjcppLog::err(TAG, "Not found webhook " + sWebhookId);
            continue;
        }

        std::chrono::time_point<std::chrono::system_clock> start_all, end_all, start, end;
        start_all = std::chrono::system_clock::now();
        std::vector<WebhookShellCommand> vCommands = webhook.getCommands();
        for (int i = 0; i < vCommands.size(); i++) {
            start = std::chrono::system_clock::now();

            DoRunCommand process(webhook.getWorkDir(), vCommands[i].getArgs());
            process.start(webhook.getTimeoutCommand()*1000);

            if (process.isTimeout()) {
                WsjcppLog::err(TAG, "Comamnd finished by timeout " + sWebhookId);
                WsjcppLog::err(TAG, process.outputString());
                continue;
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
            end = std::chrono::system_clock::now();

            int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
            WsjcppLog::info(TAG, "Elapsed milliseconds: " + std::to_string(elapsed_milliseconds) + "ms");
        }
        end_all = std::chrono::system_clock::now();
        int elapsed_milliseconds2 = std::chrono::duration_cast<std::chrono::milliseconds>(end_all - start_all).count();
        WsjcppLog::info(TAG, "Elapsed for all milliseconds: " + std::to_string(elapsed_milliseconds2) + "ms");
    }
}
