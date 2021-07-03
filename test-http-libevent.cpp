#include <stdlib.h>
#include <stdio.h>
#include <iostream>
 #include <unistd.h>

#include "event2/http.h"
#include "event2/event.h"
#include "event2/buffer.h"

#include "locallog.h"
#include "configure.h"
#include "container.h"
#include "tcpserver.h"
#include "timeoutserver.h"

using namespace std;

static std::string s_confFilePath;


bool readRunArg(int argc, char** argv)
{
	int opt = 0;
	while((opt = getopt(argc, argv, "c:h")) != -1)
	{
		switch(opt) 
		{
			case 'h':
			{
				printf("Usage: remote-server -c/etc/remote/server.conf\n");
				return false;
			}
			break;

			case 'c':
			{
				s_confFilePath = std::string(optarg);
				return true;	
			}
			break;
		}
	}
	return false;
}

int main(int argc, char** argv)
{
	bool res = readRunArg(argc, argv);
	if (false == res)
	{
		printf("Usage: remote-server -c /etc/remote/server.conf\n");
		return 0;
	}

    TcpContainer* tank = new TcpContainer(); 

	tank->loadConfigFile(s_confFilePath);
    tank->addServer(new TcpServer(tank));
    tank->loop();

    std::cout<<"i am blocked"<<endl;
    while (1);

    return 0;
}






void HttpGenericCallback(struct evhttp_request* request, void* arg)
{
    const struct evhttp_uri* evhttp_uri = evhttp_request_get_evhttp_uri(request);
    char url[8192];
    evhttp_uri_join(const_cast<struct evhttp_uri*>(evhttp_uri), url, 8192);

    printf("accept request url:%s\n", url);

    struct evbuffer* evbuf = evbuffer_new();
    if (!evbuf)
    {
        printf("create evbuffer failed!\n");
        return ;
    }

    evbuffer_add_printf(evbuf, "Server response. Your request url is %s", url);
    evhttp_send_reply(request, HTTP_OK, "OK", evbuf);
    evbuffer_free(evbuf);
}


#if 0
    if (argc != 2)
    {
        printf("usage:%s port\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port == 0)
    {
        printf("port error:%s\n", argv[1]);
        return 1;
    }

    struct event_base* base = event_base_new();
    if (!base)
    {
        printf("create event_base failed!\n");
        return 1;
    }

    struct evhttp* http = evhttp_new(base);
    if (!http)
    {
        printf("create evhttp failed!\n");
        return 1;
    }

    if (evhttp_bind_socket(http, "0.0.0.0", port) != 0)
    {
        printf("bind socket failed! port:%d\n", port);
        return 1;
    }

    evhttp_set_gencb(http, HttpGenericCallback, NULL);

    event_base_dispatch(base);
#endif

#if 0

    LocalLog* log = new LocalLog();

    log->turnOn("./log.txt");

    log->userLog(LOG_LEVEL_ALL, "yangpan %s huangbixia", "love");

    log->turnOff();

    Configure::dump("./config.txt");

#endif