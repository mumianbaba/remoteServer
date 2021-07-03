#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <mutex>

#include "event2/event.h"

#include "server.h"
#include "configure.h"
#include "session.h"


using namespace std;

class Container {

public:
	Container(){
		_servers.clear();
		_base = NULL;
	};

	virtual ~Container(){
		for (unsigned int i=0; i<_servers.size(); i++) {
			delete _servers[i];
		}
		_servers.clear();
		if (_base) {
			event_base_free(_base);
		}
	}

	/* libevent main struct */
	struct event_base *getEventBase() {
		return _base;
	}
	void setEventBase(struct event_base *base) {
		_base = base;
	}

	/* get a chance to child */
	virtual void loop() = 0;
	virtual void initContainer()=0;
	
	void addServer(Server *server) {
		_servers.push_back(server);
	}

	Server *getServer(unsigned int idx) {
		if (idx >= _servers.size())
			return NULL;
		return _servers[idx];
	}

	void startServers(struct event_base *base) {
		for (unsigned int i=0; i<_servers.size(); i++) {
			_servers[i]->run(base);
		}
	}

private:
	struct event_base* _base;
	vector<Server *> _servers;
};


//sub class
class TcpContainer : public Container {

public:
	TcpContainer(){
		initContainer();
		m_bsMap.clear();
		m_fsMap.clear();
	};

	virtual ~TcpContainer() {
		delete m_config;
	};

public:
	void loadConfigFile(const std::string& path);
	void loop();
	Session* find(struct bufferevent* bev);
	Session* find(evutil_socket_t fd);
	Session* getPeer(Session* se);
	int insert(struct bufferevent* bev,  Session* se);
	int match(Session* se);
	int removeSession(struct bufferevent* bev);
	void removeSessionAll();

private:
	void initContainer();
	int insert(evutil_socket_t fd,  Session* se);
	int match(evutil_socket_t fd);
private:
	
	map<struct bufferevent*, Session*> m_bsMap;
	map<evutil_socket_t, Session*> m_fsMap;
	Configure *m_config;
	mutex m_mut;

};
