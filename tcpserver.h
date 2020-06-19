#pragma once

#include "server.h"
#include "event2/event_struct.h"
#include "event2/bufferevent.h"
#include "event2/event.h"

#include "session.h"

#include <vector>
#include <map>
#include <iostream>
#include "container.h"

using namespace std;


struct MsgPackage{
	struct bufferevent *bev;
	TcpContainer* tc;
};


class TcpServer : public Server
{
public:
	TcpServer(TcpContainer* container);
	~TcpServer();

    /* virtual child func of server */
	int listen(struct event_base* base);

    /* virtual chile func of server */
    virtual MessageHandler *createMsgHandler();

    /* use the message handler to handle the mesaage */
    int processMessage(struct bufferevent *bev);

    /* is for libevent */
	struct event_base *getEventBase() {
		return m_base;
	}

	TcpContainer* getContainer(){
		return m_container;
	}

private:
	//struct bufferevent *bev;

	map<struct bufferevent*, Session*> m_bsMap;
	struct evconnlistener *listener;
	struct event_base *m_base;
	TcpContainer      *m_container;  
};

