#pragma once

#include "messagehandler.h"


class Server
{
public:
	Server();
    /* virtual is vary important */
	virtual ~Server(void);

    /* create father class */
	void run(struct event_base *base) {
		m_msgHandler = createMsgHandler();
		listen(base);
	};

    /* use father class and call child func */
	MessageHandler *getMsgHandler() {
		return m_msgHandler;
	};

     /* give a chance to server child */
	virtual int listen(struct event_base* base) = 0;

    /* give a chance to msg hander child */
	virtual MessageHandler *createMsgHandler() = 0;
private:
    /* father porint */
	MessageHandler *m_msgHandler;    
};

