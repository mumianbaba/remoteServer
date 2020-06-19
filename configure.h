#pragma once

#include <map>
#include <string>
#include <fstream>

using namespace std;


#define CONFIG_FILE_PATH "./config/data.cfg"

#define SERVER_HOST  "192.168.99.117"
#define SERVER_STATIC_PORT  "50000"
/* 30s */
#define SERVER_LOGIN_TIMEOUT  "30"  
/* 300s */
#define SERVER_MATCH_TIMEOUT  "300"
/* 1200s */
#define SERVER_ACTIVE_TIMEOUT  "1200"
/* 100qps */
#define SERVER_MAX_QPS         "100"
/* 100 connections */
#define SERVER_MAX_CONNECT     "100"
/* sleep 200ms, then busy */
#define SERVER_SLEEP_MILL      "200"

#define SERVER_LOG_LEVEL      "3"


#define LOG_SWITCH_NAME		"log_switch" 
#define LOG_FILE			"logfile"
#define LOG_DEFAULT_NAME    "./event.log"


class Configure
{
public:
	~Configure() {
		m_cfg = NULL;
	};
	static Configure *readConfigFile(const char *filePath);
	string &getValue(const char *key);
	static void dump(const char *filePath);
	string m_host;
	unsigned short m_staticPort;
	int m_loginTimeout;
	int m_matchTimeout;
	int m_activeTimeout;
	int m_maxQps;
	int m_maxConnect;
	int m_sleepMill;
	int m_loglevel;
private:
	Configure(){ 
		m_info["host"] = SERVER_HOST;
		m_info["static_port"] = SERVER_STATIC_PORT;
		m_info["login_timeout"] = SERVER_LOGIN_TIMEOUT;
		m_info["match_timeout"] = SERVER_MATCH_TIMEOUT;
		m_info["active_timeout"] = SERVER_ACTIVE_TIMEOUT;

		m_info["max_qps"] = SERVER_MAX_QPS;
		m_info["max_connect"] = SERVER_MAX_CONNECT;
		m_info["sleep_mill"] = SERVER_SLEEP_MILL;
		m_info["log_level"] = SERVER_LOG_LEVEL;
	};

	static void initMember(Configure *cfg);
	static void readCommonConfig(Configure *cfg, string &line);

	int ToInt(string &val);
	unsigned short ToShort(string &val);
	unsigned int ToUint(string &val);

	map<string, string> m_info;
	
	static Configure *m_cfg;
};

