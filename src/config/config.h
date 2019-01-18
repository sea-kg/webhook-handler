#ifndef CONFIG_H
#define CONFIG_H

#include <webhook.h>
#include <vector>

class Config {
	public:
		Config(const std::string &sWorkspaceDir);
		bool applyConfig();

		const std::vector<Webhook> &webhooksConf();
		int httpPort();
		int threadsForScripts();
		int maxDequeWebhooks();
		int sleepBetweenRunScriptsInSec();

	private:
		bool applyServerConf();
		bool applyWebhooksConf();
		
		std::string TAG;
		int m_nHttpPort;
		int m_nThreadsForScripts;
		int m_nMaxDequeWebhooks;
		int m_nSleepBetweenRunScriptsInSec;
		
		std::string m_sWorkspaceDir;
		std::vector<Webhook> m_vWebhooksConf;
};

#endif // CONFIG
