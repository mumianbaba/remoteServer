#include <string.h>
#include <stdio.h>

#include "event2/event.h"
#include "event2/bufferevent.h"
#include "event2/listener.h"
#include "event2/buffer.h"
#include "event2/util.h"
#include "event2/event_struct.h"
#include "event2/bufferevent_compat.h"

#include "tcpserver.h"
#include "tcpmsghandler.h"
#include "configure.h"
#include "locallog.h"
#include "connectmanager.h"
#include "threadable.h"
#include "configure.h"
#include "container.h"

TcpServer::TcpServer(TcpContainer* container) {
	listener = NULL;
	m_base = NULL;
	m_container = container;
}

TcpServer::~TcpServer() {
	if (m_container){
		m_container->removeSessionAll();
	}

	if (listener != NULL) {
		evconnlistener_free(listener);
	}

	m_container = NULL;
	LocalLog::userLog(LOG_LEVEL_ALL, "TcpServer exit ...");
}

static void conn_readcb(struct bufferevent *bev, void *user_data)
{
	if (bev == NULL || user_data == NULL) {
		LocalLog::userLog(LOG_LEVEL_ERR, "error:conn_readcb bev or user_data is null.");
		return;
	}

	TcpServer *server = (TcpServer *)user_data;

	struct evbuffer *input = bufferevent_get_input(bev);
	if (evbuffer_get_length(input) == 0) {
		LocalLog::userLog(LOG_LEVEL_ERR, "error:input data len is 0");
		bufferevent_free(bev);
	}

	server->processMessage(bev);

//Configure *cfg = Configure::readConfigFile(CONFIG_FILE_PATH);
//	ConnectionManager *cmg = ConnectionManager::getInstance();
//	cmg->recycleConnection();
}


static void conn_eventcb(struct bufferevent *bev, short events, void *user_data){

	if (bev == NULL || user_data == NULL) {
		LocalLog::userLog(LOG_LEVEL_ERR, "error:conn_eventcb bev user_data is NULL.");
		return;
	}

	TcpServer *server = (TcpServer *)user_data;
	bool close = false;
	TcpContainer* tc = server->getContainer();
	Session* se = NULL;
	Session* peer_se = NULL; 
	UCard card;
	const char* client_type = "raw";
	if(!tc){
		LocalLog::userLog(LOG_LEVEL_ERR, "error:conn_eventcb tcp container is NULL.");
		return;
	}

	if (events & BEV_EVENT_EOF || events & BEV_EVENT_ERROR ||
		events & BEV_EVENT_TIMEOUT)
	 {
		se = tc->find(bev);
		peer_se = tc->getPeer(se);
		if (!se){
			bufferevent_free (bev);
			LocalLog::userLog(LOG_LEVEL_ERR,"note:conn_eventcb Connection close, but can't find session");
			return;
		}
		memset(&card, 0, sizeof(card));
		se->getUserData(0, &card, sizeof(card));
		if (card.type == 1){
			client_type = "cpe device";
		}
		else if (card.type == 2){
			client_type = "remote client";
		}
	 }

	if (events & BEV_EVENT_EOF) {
		LocalLog::userLog(LOG_LEVEL_ALL, "note:conn_eventcb %s Connection close.", client_type);
		close = true;
	} 
	else if (events & BEV_EVENT_ERROR) {
		LocalLog::userLog(LOG_LEVEL_ALL, "error:conn_eventcb Got an error on the connection:%s", client_type);
		close = true;
	}
	else if(events & BEV_EVENT_TIMEOUT ){
		const char* action = (events | BEV_EVENT_READING)? "Read" : "Write";
		LocalLog::userLog(LOG_LEVEL_DBG, "note:conn_eventcb %s:%s Timeout... %d", client_type, action, time(NULL));
		close = true;
		if (se){
			if (se->inactive(40) == 0){
				close = false;
			}
		}
		if (peer_se){
			if (peer_se->inactive(0) == 0){
				close = false;
			}
		}
		if (events | BEV_EVENT_READING){
			bufferevent_enable (bev, EV_READ);
		}
		if (events | BEV_EVENT_WRITING){
			bufferevent_enable (bev, EV_WRITE);
		}
	}

	if (close){
		if (se){
			se->dump_info();
		}
		if (peer_se){
			peer_se->dump_info();
		}
		tc = server->getContainer();
		tc->removeSession(bev);
	}
}


static void	listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
						struct sockaddr *sa, int socklen, void *arg) 
{
	int ret = 0;
	TcpServer *server = (TcpServer *)arg;
	struct bufferevent *bev;

	bev = bufferevent_socket_new(server->getEventBase(), fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) {
		LocalLog::userLog(LOG_LEVEL_ERR, "error:listener_cb constructing bufferevent!");
		event_base_loopbreak(server->getEventBase());
		return;
	}

	Session* se = new Session();
	if (!se){
		LocalLog::userLog(LOG_LEVEL_ERR, "error:listener_cb new Session failed!");
		return;
	}
	se->setBev(0, bev);

	TcpContainer* tc = server->getContainer();
	if (tc){
		ret = tc->insert(bev, se);
		if (ret != 0){
			delete se;
			se = NULL;
			LocalLog::userLog(LOG_LEVEL_ERR, "error:listener_cb setBevSessionPair failed!");
			return;
		}
	}

	struct timeval tv;
	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = 40;
	tv.tv_usec = 0;
	bufferevent_set_timeouts(bev, &tv, &tv);

	bufferevent_setcb(bev, conn_readcb, NULL, conn_eventcb, arg);
	bufferevent_enable(bev, EV_WRITE | EV_READ);

	//first write
	Configure *cfg = Configure::readConfigFile(CONFIG_FILE_PATH);
	ret = bufferevent_write(bev, cfg->m_host.c_str(), cfg->m_host.length());
	if (ret != 0) {
		LocalLog::userLog(LOG_LEVEL_ERR, "error:server send data failed");
	}
	LocalLog::userLog(LOG_LEVEL_ALL, "a new connection established!");
}


int TcpServer::listen(struct event_base* base) {
	
	struct sockaddr_in sin;

	Configure *cfg = Configure::readConfigFile(CONFIG_FILE_PATH);

	this->m_base = base;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(cfg->m_staticPort);
	sin.sin_addr.s_addr = INADDR_ANY;
	
	Configure::dump(CONFIG_FILE_PATH);
	
	listener = evconnlistener_new_bind(m_base, listener_cb, this,
		         LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
		         (struct sockaddr*)&sin,
		         sizeof(sin));

	if (!listener) {
		LocalLog::userLog(LOG_LEVEL_ERR, "error:Could not create a listener!");
		return 1;
	}

	LocalLog::userLog(LOG_LEVEL_ALL, "TcpServer start ...");
	
	return 0;
}

MessageHandler *TcpServer::createMsgHandler() {
	return new TcpServerMsgHandler();
}

int TcpServer::processMessage(struct bufferevent *bev) {

	MessageHandler *handler = getMsgHandler();
	if (handler == NULL) {
		LocalLog::userLog(LOG_LEVEL_ERR, "error:TcpServer getMsgHandler failed.");
		return -1;
	}

	ConnectionManager *cmg = ConnectionManager::getInstance();
	cmg->requestInc();
	Configure *cfg = Configure::readConfigFile(CONFIG_FILE_PATH);
	if (cfg != NULL) {
		int conn = cmg->getConnections();
		int req_per = cmg->getCurrentRequest();
		if ((cfg->m_maxConnect != -1 && conn >= cfg->m_maxConnect) 
			|| (cfg->m_maxQps != -1 && req_per >= cfg->m_maxQps)) {
			Threadable::milliSleep(10);
		}
	}

	MsgPackage msg;
	memset(&msg, 0, sizeof(msg));

	msg.bev = bev;
	msg.tc = m_container;
	return handler->processMessage((void*)&msg);
}
