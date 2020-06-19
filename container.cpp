#include <iostream>
#include <string.h>

#include <event2/http_compat.h>

#include "container.h"
#include "connectmanager.h"
#include "locallog.h"
#include "configure.h"



void TcpContainer::initContainer() {
	setEventBase(event_base_new());

	m_config = Configure::readConfigFile(CONFIG_FILE_PATH);
	string logswitch = m_config->getValue(LOG_SWITCH_NAME);
    string logfile = LOG_DEFAULT_NAME;
	if (!logswitch.empty()) {
		logfile = m_config->getValue(LOG_FILE);
		if (logfile.empty()) {
			logfile = LOG_DEFAULT_NAME;
		}
	}
    LocalLog::turnOn(logfile.c_str());
};

void TcpContainer::loop() {
	int ret;
    startServers(getEventBase());
	ret = event_base_dispatch(getEventBase());
    LocalLog::userLog(LOG_LEVEL_ALL, "note:%s event_base_dispatch return %d", (ret == -1)? "error:":"note:", ret);
};


Session* TcpContainer::find(struct bufferevent* bev){
	std::lock_guard<std::mutex> lk(m_mut);
	map<struct bufferevent*, Session*>::iterator it;
	it = m_bsMap.find(bev);
	if (it == m_bsMap.end())
	{
		LocalLog::userLog(LOG_LEVEL_DBG, "note: no find the bev");
		return NULL;
	}
	return it->second;
}

Session* TcpContainer::find(evutil_socket_t fd){

	std::lock_guard<std::mutex> lk(m_mut);
	map<evutil_socket_t, Session*>::iterator it;
	it = m_fsMap.find(fd);
	if (it == m_fsMap.end())
	{
		LocalLog::userLog(LOG_LEVEL_DBG, "note: no find the bev");
		return NULL;
	}
	return it->second;
}

Session* TcpContainer::getPeer(Session* se){
	if (!se){
		LocalLog::userLog(LOG_LEVEL_ERR, "error:getPeer se null");
		return NULL;
	}
	{
		std::lock_guard<std::mutex> lk(m_mut);
		map<struct bufferevent*, Session*>::iterator it;

		struct bufferevent * bf = se->getBev(1);
		it = m_bsMap.find(bf);
		if (it == m_bsMap.end())
		{
			LocalLog::userLog(LOG_LEVEL_DBG, "note: no find peer the bev");
			return NULL;
		}
		return it->second;
	}
}


int TcpContainer::insert(struct bufferevent* bev,  Session* se){
	
	if (!bev || !se){
		LocalLog::userLog(LOG_LEVEL_ERR, "error:insert  bev se null");
		return -1;
	}

	{
		std::lock_guard<std::mutex> lk(m_mut);

		pair<map<struct bufferevent *, Session*>::iterator, bool> res;
		
		res = m_bsMap.insert(pair<struct bufferevent *, Session*>(bev, se));
		if (!res.second){
			cout<<"inset bev session err"<<endl;
			LocalLog::userLog(LOG_LEVEL_DBG, "error:insert  inset bev session pair err");
			return -1;
		}
		else{
			cout<<"inset bev session pair successful"<<endl;
		}
		
		pair<map<evutil_socket_t, Session*>::iterator, bool> res2;
		evutil_socket_t fd = bufferevent_getfd(bev);
		res2 = m_fsMap.insert(pair<evutil_socket_t, Session*>(fd, se));
		if (!res2.second){
			LocalLog::userLog(LOG_LEVEL_ERR, "error:insert  inset fd session pair err");
		}
	}
	return 0;
}

int TcpContainer::match(Session* se){
	
	int ret = -1;
	std::lock_guard<std::mutex> lk(m_mut);
	map<struct bufferevent*, Session*>::iterator it;
	/* login successful */
	for (it = m_bsMap.begin(); it != m_bsMap.end(); it++){
		ret = se->match(it->second);
		if (ret == 0){
			LocalLog::userLog(LOG_LEVEL_DBG, "processMessage: match successful");
			/* write log session info */
        	se->dump_info();
			it->second->dump_info();
			break;
		}
	}
	return ret;
}

int TcpContainer::insert(evutil_socket_t fd,  Session* se){
	
	std::lock_guard<std::mutex> lk(m_mut);
	pair<map<evutil_socket_t, Session*>::iterator, bool> res;
	res = m_fsMap.insert(pair<evutil_socket_t, Session*>(fd, se));
	if (!res.second){
		LocalLog::userLog(LOG_LEVEL_ERR, "error:insert  inset fd session pair err");
		return -1;
	}
	return 0;
}

int TcpContainer::removeSession(struct bufferevent* bev){
	if (!bev){
		LocalLog::userLog(LOG_LEVEL_ERR, "error:removeSession  bev null");
		return -1;
	}

	int ret = 0;
	Session* se = NULL;
	struct bufferevent* peerBev = NULL;
	UCard card;
	memset(&card, 0, sizeof(card));
	{
		std::lock_guard<std::mutex> lk(m_mut);
		map<struct bufferevent*, Session*>::iterator it;

		evutil_socket_t fd = bufferevent_getfd(bev);
		if (fd > 0){
			m_fsMap.erase(fd);
		}
		
		it = m_bsMap.find(bev);
		if (it == m_bsMap.end()){
			LocalLog::userLog(LOG_LEVEL_DBG, "note:removeSession no find the bev");
			bufferevent_free(bev);
			return 0;
		}

		se = it->second;
		peerBev = se->getBev(1); 

		if (se){
			ret = se->getUserData(0,&card, sizeof(card));
			const char* type = "raw";
			if (card.type == 1){
				type = "cpe device";
			}
			else if (card.type == 2){
				type = "remote client";
			}
			LocalLog::userLog(LOG_LEVEL_DBG, "note:removeSession delete %s session id:%s\n", 
			                    type, card.id);
			delete se;
			se = NULL;
		}
		m_bsMap.erase(bev);
		if (!peerBev){
			return 0 ;
		}

		fd = bufferevent_getfd(peerBev);
		if(fd > 0){
			m_fsMap.erase(fd);
		}
		it = m_bsMap.find(peerBev);
		if (it == m_bsMap.end()){
			/* can't find the the bev */
			bufferevent_free(peerBev);
			return 0;
		}

		se =it->second;
		if (se){
			
			ret = se->getUserData(0,&card, sizeof(card));
			const char* type = "raw";
			if (card.type == 1){
				type = "telnetd";
			}
			else if (card.type == 2){
				type = "telnetc";
			}
			LocalLog::userLog(LOG_LEVEL_DBG, "note:removeSession delete %s session id:%s\n", 
			                    type, card.id);
			delete se;
			se = NULL;
		}
		m_bsMap.erase(peerBev);
	}
	return 0;
}


void TcpContainer::removeSessionAll(){

	map<struct bufferevent*, Session*>::iterator it;
	{
		std::lock_guard<std::mutex> lk(m_mut);
		for(it = m_bsMap.begin(); it != m_bsMap.end(); it++){
			if (it->second){
				delete it->second;	
			}
		}
		m_bsMap.erase( m_bsMap.begin(), m_bsMap.end());
		m_fsMap.erase(m_fsMap.begin(), m_fsMap.end());
	}
	return;
}