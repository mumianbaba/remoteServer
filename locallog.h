#pragma once

#include <stdio.h>


#define LOG_TEMP_BUFFER_SIZE 1024


enum LOG_LEVEL{
	LOG_LEVEL_ALL = 0,
	LOG_LEVEL_ERR = 1,
	LOG_LEVEL_DBG = 2,
	LOG_LEVEL_VERBOSE = 3,
	LOG_LEVEL_PRT = 4,
};

class LocalLog
{
public:
	LocalLog(void);
	~LocalLog(void);

	static void turnOn(const char *filepath);
	static void turnOff();
	static void userLog(LOG_LEVEL level, const char *fmt, ...);

    //static FILE* m_logfile;
    static int* a; 
};
