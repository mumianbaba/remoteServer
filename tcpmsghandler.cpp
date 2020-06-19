#include <iostream>
#include <map>

#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include "event2/listener.h"
#include "event2/util.h"
#include "event2/event_struct.h"

#include "tcpserver.h"
#include "tcpmsghandler.h"
#include "locallog.h"
#include "session.h"
#include "container.h"



using namespace std;

int TcpServerMsgHandler::processMessage(void *arg) {

	int ret = 0;
	MsgPackage * msg = (MsgPackage *)arg;

	if (!msg || !msg->bev || !msg->tc){
		LocalLog::userLog(LOG_LEVEL_ERR, "error:processMessage msg or bev or bsMap null");
		return -1;
	}

	struct bufferevent *bev = msg->bev;
	TcpContainer* tc = msg->tc;
	Session* se = tc->find(bev);
	if (!se){
		/* not find the sessiom the map of bev */
		LocalLog::userLog(LOG_LEVEL_ERR, "error:processMessage cant  find the sessiom");
		return -1;		
	}

	char data[4096] = {0};
	int len = bufferevent_read(bev, data, sizeof(data));
	if (len <= 0) {
		LocalLog::userLog(LOG_LEVEL_ERR, "error:processMessage len less than 0");
		return -1;
	}

	//data[len] = '\0';
	//LocalLog::userLog(LOG_LEVEL_PRT,"processMessage:%s", data);

	SessionState state = se->getSessionState();
	if (state == SStateRaw){
		ret = se->login(data, len);
		LocalLog::userLog(LOG_LEVEL_ALL, "note:processMessage login ret=%d", ret);
		/* login successful */
		if (ret == 0){
			ret = tc->match(se);
			if (ret == 0){
				/* bind successful */
				struct bufferevent * bf0 = NULL;
				struct bufferevent * bf1 = NULL;
				bf0 = se->getBev(0);
				bf1 = se->getBev(1);
				if (bf0 && bf1){
					ret = bufferevent_write(bf0, TELPROXY_OK, strlen(TELPROXY_OK));
					LocalLog::userLog(LOG_LEVEL_ALL, "note:processMessage match succ and send ok 0 ret=%d", ret);
					ret = bufferevent_write(bf1, TELPROXY_OK, strlen(TELPROXY_OK));
					LocalLog::userLog(LOG_LEVEL_ALL, "note:processMessage match succ and send ok 1 ret=%d", ret);					
				}
			}
		}
	}
	else if (state == SStateBind){
		ret = se->business(data, len);
		LocalLog::userLog(LOG_LEVEL_VERBOSE, "sid:%d send message to peer,ret=%d", se->getSid(), ret);
	}
	else if (state == SStateDie){
		LocalLog::userLog(LOG_LEVEL_VERBOSE, "sid:%d the session is dead", se->getSid());
	}
	else if (state == SStateLogin){
		LocalLog::userLog(LOG_LEVEL_VERBOSE, "sid:%d the session is just login, not bind", se->getSid());
	}else 
	{
		LocalLog::userLog(LOG_LEVEL_ERR, "sid:%d the session is just login, not bind", se->getSid());
	}
	return 0;
}