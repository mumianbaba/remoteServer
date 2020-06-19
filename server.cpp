#include <stdio.h>


#include "server.h"


Server::Server() {
	m_msgHandler = NULL;
}


Server::~Server(void) {
	delete m_msgHandler;
}
