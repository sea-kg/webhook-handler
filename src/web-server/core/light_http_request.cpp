#include "light_http_request.h"
#include <logger.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <sys/time.h>
#include <fstream>
#include <regex>        // regex, sregex_token_iterator
#include <stdio.h>
#include <math.h>
#include <fs.h>
#include <ts.h>
#include <light_http_response.h>

// ----------------------------------------------------------------------

LightHttpRequest::LightHttpRequest(int nSockFd, const std::string &sAddress) {
	m_nSockFd = nSockFd;
	m_sAddress = sAddress;
	m_bClosed = false;
	TAG = "LightHttpRequest";
	long nSec = TS::currentTime_seconds();
	m_sLastModified = TS::formatTimeForWeb(nSec);
}

// ----------------------------------------------------------------------

int LightHttpRequest::sockFd(){
	return m_nSockFd;
}

// ----------------------------------------------------------------------

std::string LightHttpRequest::requestType(){
	return m_sRequestType;
}

// ----------------------------------------------------------------------

std::string LightHttpRequest::requestPath(){
	return m_sRequestPath;
}

// ----------------------------------------------------------------------

std::string LightHttpRequest::requestHttpVersion(){
	return m_sRequestHttpVersion;
}

std::map<std::string,std::string> &LightHttpRequest::requestQueryParams() {
	return m_sRequestQueryParams;
}

// ----------------------------------------------------------------------

std::string LightHttpRequest::address(){
	return m_sAddress;
}

// ----------------------------------------------------------------------

void LightHttpRequest::parseRequest(const std::string &sRequest){
    std::istringstream f(sRequest);
	std::string firstLine = "";
    if (getline(f, firstLine, '\n')) {
        // nothing
    }

	if (firstLine.size() > 0) {
		std::istringstream f(firstLine);
		std::vector<std::string> params;
		std::string s;
		while (getline(f, s, ' ')) {
			params.push_back(s);
		}
		if(params.size() > 0){
			m_sRequestType = params[0];
		}

		if(params.size() > 1){
			m_sRequestPath = params[1];
		}

		// TODO m_sRequestPath - need split by ? if exists
		if(params.size() > 2){
			m_sRequestHttpVersion = params[2];
		}
	}

	// parse query
	std::size_t nFound = m_sRequestPath.find("?");
  	if (nFound != std::string::npos) {
		std::string sQuery = m_sRequestPath.substr(nFound+1);
		m_sRequestPath = m_sRequestPath.substr(0, nFound);
		std::istringstream f(sQuery);
		std::string sParam;
		while (getline(f, sParam, '&')) {
			std::size_t nFound2 = sParam.find("=");
			std::string sValue = sParam.substr(nFound2+1);
			std::string sName = sParam.substr(0, nFound2);
			m_sRequestQueryParams[sName] = sValue;
		}
	}
}
