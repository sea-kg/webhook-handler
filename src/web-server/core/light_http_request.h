#ifndef LIGHT_HTTP_REQUEST_H
#define LIGHT_HTTP_REQUEST_H

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <map>

class LightHttpRequest {
	public:
		LightHttpRequest(
			int nSockFd,
			const std::string &sAddress
		);
		~LightHttpRequest(){};

		int sockFd();
		void parseRequest(const std::string &sRequest);

		
		std::string address();
		std::string requestType();
		std::string requestPath();
		std::string requestHttpVersion();
		std::map<std::string,std::string> &requestQueryParams();

	private:
		std::string TAG;

		int m_nSockFd;
		bool m_bClosed; 
		std::string m_sAddress;
		std::string m_sRequestType;
		std::string m_sRequestPath;
		std::string m_sRequestBody;
		std::map<std::string,std::string> m_sRequestQueryParams;
		std::string m_sRequestHttpVersion;

		std::string m_sResponseCacheControl;
		std::string m_sLastModified;
};

#endif // LIGHT_HTTP_REQUEST_H