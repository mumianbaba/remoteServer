#pragma once


class MessageHandler
{
public:
	MessageHandler(void);
	virtual ~MessageHandler(void);

	virtual int processMessage(void *arg) = 0;
};
