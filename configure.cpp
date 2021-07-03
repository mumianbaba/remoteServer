#include "configure.h"
#include <iostream>

using namespace std;


Configure *Configure::m_cfg = NULL;

std::string Configure::m_confFilePath;


static void repaceTab(string &str) {

	for (unsigned int i=0; i<str.length(); i++) {
		if (str[i] == '\t'|| str[i] == '\b' || str[i] == '\v' || str[i] == '\r') {
			str[i] = ' ';
		}
	}
}

static string& trim(string& str){

	str.erase(str.find_last_not_of(' ')+1, string::npos);
	str.erase(0, str.find_first_not_of(' '));
	return str;
}

void Configure::initMember(Configure *cfg) {

	m_cfg->m_host = m_cfg->m_info["host"];
	m_cfg->m_staticPort = m_cfg->ToShort(m_cfg->m_info["static_port"]);
	m_cfg->m_loginTimeout = m_cfg->ToInt(m_cfg->m_info["login_timeout"]);
	m_cfg->m_matchTimeout = m_cfg->ToInt(m_cfg->m_info["match_timeout"]);
	m_cfg->m_activeTimeout = m_cfg->ToInt(m_cfg->m_info["active_timeout"]);

	m_cfg->m_maxQps = m_cfg->ToInt(m_cfg->m_info["max_qps"]);
	m_cfg->m_maxConnect = m_cfg->ToInt(m_cfg->m_info["max_connect"]);
	m_cfg->m_sleepMill = m_cfg->ToInt(m_cfg->m_info["sleep_mill"]);
	m_cfg->m_loglevel = m_cfg->ToInt(m_cfg->m_info["log_level"]);
}

void Configure::readCommonConfig(Configure *cfg, string &line) {

	string::size_type index = line.find_first_of(" ", 0);
	string key = line.substr(0, index);
	string value = line.substr(index+1);
	m_cfg->m_info[trim(key)] = trim(value);
}

Configure *Configure::readConfigFile() {
	if (!m_confFilePath.empty())
	{
		return readConfigFile(m_confFilePath);
	}
	return NULL;
}


Configure *Configure::readConfigFile(const std::string &filePath) {
	
	if (m_cfg != NULL) {
		return m_cfg;
	}

	m_cfg = new Configure();
	ifstream ios(filePath, ios::in);
	if (!ios.good()) {
		cout << "open configure file failed." << endl;
		initMember(m_cfg);
		return m_cfg;
	}

	int flag = 0;
	while (!ios.eof()) {
		string line;
		getline(ios, line);
		repaceTab(line);
		readCommonConfig(m_cfg, line);
	}
	initMember(m_cfg);

	m_confFilePath = filePath;
	return m_cfg;
}

string& Configure::getValue(const char *key) {
	return m_info[key];
}

int Configure::ToInt(string &val) {
	return atoi(val.c_str());
}

unsigned short Configure::ToShort(string &val) {
	return (unsigned short)ToInt(val);
}

unsigned int Configure::ToUint(string &val) {
	return (unsigned int)ToInt(val);
}


void Configure::dump()
{
    Configure* cfg = Configure::readConfigFile();
	if (NULL == cfg){
		std::cout<<"read config file failed"<<std::endl;
		return;
	}

    string param = cfg->getValue("host");
    cout<<"host:"<<param<<endl;

    param = cfg->getValue("static_port");
    cout<<"static_port:"<<param<<endl;

    param = cfg->getValue("login_timeout");
    cout<<"login_timeout:"<<param<<endl;

    param = cfg->getValue("match_timeout");
    cout<<"match_timeout:"<<param<<endl;

    param = cfg->getValue("max_qps");
    cout<<"max_qps:"<<param<<endl;

    param = cfg->getValue("max_connect");
    cout<<"max_connect:"<<param<<endl;
    
    param = cfg->getValue("sleep_mill");
    cout<<"sleep_mill:"<<param<<endl;
}
