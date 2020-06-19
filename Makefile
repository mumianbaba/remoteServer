
LIBEVENT_INC := $(LIBEVENT_PATH)/include
LIBEVENT_LIBS :=  $(LIBEVENT_PATH)/lib/libevent.a

SOURCE :=  test-http-libevent.cpp locallog.cpp
SOURCE +=  configure.cpp
SOURCE +=  server.cpp
SOURCE +=  messagehandler.cpp
SOURCE +=  connectmanager.cpp
SOURCE +=  tcpserver.cpp
SOURCE +=  tcpmsghandler.cpp
SOURCE +=  threadable.cpp
SOURCE +=  container.cpp
SOURCE +=  session.cpp
SOURCE +=  timeoutserver.cpp


LINK := -lpthread

all:
	echo $(LIBEVENT_PATH)
	g++  $(SOURCE) $(LIBEVENT_LIBS) -I $(LIBEVENT_INC) $(LINK) -o remote-service

clean:
	rm -f *.o test-http

