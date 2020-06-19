#pragma once

#include "messagehandler.h"

class TcpServerMsgHandler : public MessageHandler
{
public:
	TcpServerMsgHandler(void){};
	virtual ~TcpServerMsgHandler(void){};

	int processMessage(void *arg);
};

