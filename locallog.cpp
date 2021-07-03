#include "locallog.h"
#include "event2/event.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include "configure.h"

using namespace std;



static FILE* m_logfile = NULL;


LocalLog::LocalLog(void)
{
	m_logfile = NULL;
}


LocalLog::~LocalLog(void)
{
	fclose(m_logfile);
}

void write_to_file_fatal_cb(int err) {
	if (m_logfile == NULL) {
		return;
	}

	fprintf(m_logfile, "[**** fatal error ****] %d\n", err);
}

static void discard_log(int severity, const char *msg) {

}

static void write_to_file_cv(int severity, const char *msg) {
	const char *s;
	if (m_logfile == NULL) {
		return;
	}

	switch (severity) {
	case _EVENT_LOG_DEBUG:
		s = "debug";
		break;
	case _EVENT_LOG_MSG:
		s = "msg";
		break;
	case _EVENT_LOG_WARN:
		s = "warn";
		break;
	case _EVENT_LOG_ERR:
		s = "error";
		break;
	default:
		s = "?";
		break;
	}

	fprintf(m_logfile, "[%s] %s\n", s, msg);
	fflush(m_logfile);
}

void LocalLog::turnOn(const char *filepath) {
	if (m_logfile != NULL) {
		event_set_log_callback(write_to_file_cv);
		return;
	}

	m_logfile = fopen(filepath, "w"); //a+
	if (m_logfile == NULL) {
		event_set_log_callback(discard_log);
	} else {
		event_set_log_callback(write_to_file_cv);
	}

	event_set_fatal_callback(write_to_file_fatal_cb);
}

void LocalLog::turnOff() {
	event_set_log_callback(discard_log);
	fclose(m_logfile);
	m_logfile = NULL;
}

void LocalLog::userLog(LOG_LEVEL level,const char *fmt, ...) {

	if (m_logfile == NULL) {
		return;
	}
	Configure *cfg = Configure::readConfigFile();
	if (NULL == cfg){
		std::cout<<"read the config failed"<<std::endl;
		return;
	}
	if (level > cfg->m_loglevel){
		return;
	}

	va_list ap;

	char buf[LOG_TEMP_BUFFER_SIZE];
	memset(buf, 0, sizeof(buf));
	time_t now;
	struct tm *curtime = NULL;
	time(&now);
	curtime = localtime(&now);
	sprintf(buf, "[%d-%02d-%02d %02d:%02d:%02d] ", (curtime->tm_year+1900), (curtime->tm_mon+1), curtime->tm_mday,
		curtime->tm_hour, curtime->tm_min, curtime->tm_sec);

	int len = strlen(buf);
	va_start(ap, fmt);
	vsnprintf(buf+len, LOG_TEMP_BUFFER_SIZE-len-1, fmt, ap);
	va_end(ap);

	fprintf(m_logfile, "%s\n", buf);
	fflush(m_logfile);

	cout<<buf+len<<endl;
	
}