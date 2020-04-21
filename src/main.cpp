#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <syslog.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <scripts_thread.h>
#include <light_http_server.h>
#include <http_handler_webhooks.h>
#include <help_parse_args.h>
#include <unistd.h>
#include <limits.h>
#include <deque_webhooks.h>
#include <wsjcpp_core.h>

LightHttpServer g_httpServer;
std::vector<ScriptsThread *> g_vThreads;

// ---------------------------------------------------------------------

void quitApp(int signum) {
  std::cout << std::endl << "Terminating server by signal " << signum << std::endl;
  g_httpServer.stop();
  exit(1);
}

// ---------------------------------------------------------------------

int main(int argc, char* argv[]) {
    std::string TAG = "MAIN";
    std::string sAppName = std::string(WSJCPP_NAME);
    std::string sAppVersion = std::string(WSJCPP_VERSION);

    HelpParseArgs helpParseArgs(argc, argv);
    helpParseArgs.setAppName(sAppName);
    helpParseArgs.setAppName(sAppVersion);

    helpParseArgs.addHelp("start", "-s", false, 
        "Start mecd service");

    helpParseArgs.addHelp("--dir", "-d", true, 
        "Workspace folder with configs, logging, scripts and etc.");

    std::string sArgsErrors;
    if (!helpParseArgs.checkArgs(sArgsErrors)){
        WsjcppLog::err(TAG, "Arguments has errors " + sArgsErrors);
        return -1;
    }

    if (helpParseArgs.handleDefaultHelps()) {
        return 0;
    }

    std::string sWorkspace = "./data"; // default workspace
    if (helpParseArgs.has("--dir")) {
        sWorkspace = helpParseArgs.option("--dir");
        // TODO check directory existing and apply dir
    }

    if (!WsjcppCore::dirExists(sWorkspace)) {
        std::cout << "Error: Folder " << sWorkspace << " does not exists \n";
        return -1;
    }

    std::string sLogDir = sWorkspace + "/logs";
    if (!WsjcppCore::dirExists(sLogDir)) {
        std::cout << "Error: Folder " << sLogDir << " does not exists \n";
        return -1;
    }

    WsjcppLog::setLogDirectory(sLogDir);
    std::cout << "Logger: '" + sWorkspace + "/logs/' \n";
    WsjcppLog::info(TAG, "Version: " + std::string(sAppVersion));

    Config *pConfig = new Config(sWorkspace);
    if (!pConfig->applyConfig()) {
        WsjcppLog::err(TAG, "Could not read config");
        return -1;
    }

    // configure http handlers

    signal( SIGINT, quitApp );
    signal( SIGTERM, quitApp );

    if (helpParseArgs.has("start")) {
        WsjcppLog::info(TAG, "Starting...");
        DequeWebhooks *pDequeWebhooks = new DequeWebhooks(pConfig->maxDequeWebhooks());

        for (int i = 0; i < pConfig->threadsForScripts(); i++) {
            ScriptsThread *thr = new ScriptsThread(pConfig, i, pDequeWebhooks);
            thr->start();
            g_vThreads.push_back(thr);
        }

        WsjcppLog::ok(TAG, "Start web-server on " + std::to_string(pConfig->httpPort()));
        g_httpServer.handlers()->add((LightHttpHandlerBase *) new HttpHandlerWebhooks(pConfig, pDequeWebhooks));
        // pConfig->setStorage(new RamStorage(pConfig->scoreboard())); // replace storage to ram for tests
        g_httpServer.start(pConfig->httpPort()); // will be block thread

        // TODO: stop all threads
        /*while(1) {
            WsjcppLog::info(TAG, "wait 2 minutes");
            std::this_thread::sleep_for(std::chrono::minutes(2));
            WsjcppLog::info(TAG, "wait ended");
        }*/
        return 0;
    }

    helpParseArgs.printHelp();
    return 0;
}
