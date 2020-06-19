#pragma once

#include <string.h>

#include "event2/event_struct.h"
#include "event2/bufferevent.h"
#include "event2/event.h"



struct UCard{
    unsigned int type;
    char id[64];
};

#define TELPROXY_TYPE_DEVICE  1
#define TELPROXY_TYPE_CLINET  2


#define TELPROXY_SYN         "remote-login"
#define TELPROXY_OK          "telnet-ok"


 struct TelProxyLogin{
     char sync[12];
     unsigned int  len;
     unsigned int type;
     char id[64];
 } __attribute__ ((packed));

struct TelProxyLoginRsp{
	char sync[12];
	unsigned int  len;
	unsigned int cid;
	unsigned int result; 
}__attribute__ ((packed));








enum SessionState{
    SStateErr = -1,
    SStateRaw,
    SStateLogin,
    SStateBind,
    SStateDie,
};

struct Meetee{
	struct bufferevent *bev;
    void* userData;
};


class Session{
public:
	Session();
	~Session();
public:
	int setBev(int index, struct bufferevent * bev);
	int setUserData(int index, void* userData, int len);

	struct bufferevent * getBev(int index);
	int getUserData(int index, void* userData, int len);

	SessionState getSessionState();
	void setSessionState(SessionState state);
	
	int getSid(){
		return 	m_sid;
	}
	int inactive(int interval);
	void clearTimeout();

	int login(const char* data, int len);
	int logout();
	int match(Session* se);
	int match_done(struct bufferevent * bev, UCard* card);
	int business(const char* data, int len);
	void dump_info(void);

private:
	static int m_sidCount;
private:
	int m_sid;
	int m_inactive;
	int m_timeouts;
	SessionState m_state;
	Meetee m_meetee[2];
};


static char* strnstr( const char * src, long src_len, const char * substr, long substr_len )
{
	char ch, chOtherCase;
	const char * p;
	const char * pend;
	
	if ( src == NULL || 0==*src || substr == NULL || 0==*substr || src_len <= 0 || substr <= 0 ) 
	{
		return NULL;
	}    	
	
	p = src;	
	pend = p + src_len - substr_len;
	while ( p < pend )		
	{		
		if ( *p == *substr )			
		{
			if ( memcmp( p, substr, substr_len ) == 0)				
			{				
				return (char*)p;				
			}
		}
		p++;
	}
	
	return NULL;	
}
