#include "timeoutserver.h"
#include "locallog.h"


#include "event2/event.h"
#include "event2/bufferevent.h"
#include "event2/listener.h"
#include "event2/buffer.h"
#include "event2/util.h"
#include "event2/event_struct.h"
#include "event2/bufferevent_compat.h"


int g_flag = 1;
int times = 0;


TimeoutServer::TimeoutServer(TcpContainer* container) {
	m_event = NULL;
    m_container = container;
}


TimeoutServer::~TimeoutServer(void) {
	if (m_event != NULL) {
		event_free(m_event);
	}
	m_container = NULL;
	LocalLog::userLog(LOG_LEVEL_ALL, "TimeoutServer exit ...");
}


static void	timeout_cb(evutil_socket_t fd, short event, void *arg) {
	if (g_flag)
		g_flag = 0;
	times++;
	if (times >= 5)
    {
        event_base_loopbreak((struct event_base *)arg);
    }
		
}


int TimeoutServer::listen(struct event_base* base) {
	
	m_event = event_new(base, -1, EV_PERSIST, timeout_cb, base);

	evutil_timerclear(&m_tv);
	m_tv.tv_sec = 5;
	m_tv.tv_usec = 0;

	int ret = event_add(m_event, &m_tv);
	if (ret) {
		printf("err init event: %d\n", ret);
		return -1;
	}
	LocalLog::userLog(LOG_LEVEL_ALL, "TimeoutServer start ...");
	return 0;
}

MessageHandler *TimeoutServer::createMsgHandler() {
	//create MessageHandler
	return NULL;
}