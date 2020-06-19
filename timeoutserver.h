#pragma once
#include "server.h"
#include <time.h>
#include "container.h"

class TimeoutServer : public Server
{
public:
	TimeoutServer(TcpContainer* container);
	~TimeoutServer(void);

	int listen(struct event_base* base);
	MessageHandler *createMsgHandler();
private:
	struct event *m_event;
	struct timeval m_tv;
    TcpContainer*  m_container;
};

