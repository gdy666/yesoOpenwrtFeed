/*
 * logger.c
 *
 *  Created on: 2015年5月5日
 *      Author: yeso
 *   非线程安全,单线程
 */
#include "logger.h"

#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

int current_level = L_TRACE;
static char* logLevStr[]={"TRACE","DEBUG","INFO","WARN","ERROR"};

//static void log_print(int log_level, const char* content , va_list ap);
static void log_print(int log_level,const char* file,const char* line, const char* content , va_list ap);


void log_set_output(const char* dest){
	int out_fd = open (dest, O_RDWR | O_CREAT | O_APPEND, 0666);
	if(out_fd<=0){
		perror("log_set_output error!");
		exit(1);
	}
	dup2(out_fd,STDOUT_FILENO);
	close(out_fd);
}

void log_set_level(int level){
	current_level = level;
}

//static void log_print(int log_level, const char* content , va_list ap){
static void log_print(int log_level,const char* file,const char* line, const char* content , va_list ap){
	if(log_level<current_level) return;

	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	struct tm *p = localtime(&tv.tv_sec);
	printf("[%d-%02d-%02d %02d:%02d:%02d.%ld]", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
#ifdef DEBUG
	printf("[%s:%d:%s]",file,line,logLevStr[log_level]);
#else
	printf("[%s]",logLevStr[log_level]);
#endif


	vprintf ( content , ap ) ;
//	va_end ( ap ) ;
	printf("\n");
}

void log_trace(const char* file,const char* line,const char* content , ...){
	va_list ap ;
	va_start ( ap , content ) ;
	log_print(L_TRACE,file,line,content,ap);
	va_end ( ap ) ;
}

void log_debug(const char* file,const char* line,const char* content , ...){
	va_list ap ;
	va_start ( ap , content ) ;
	log_print(L_DEBUG,file,line,content,ap);
	va_end ( ap ) ;
}

void log_info(const char* file,const char* line,const char* content , ...){
	va_list ap ;
	va_start ( ap , content ) ;
	log_print(L_INFO,file,line,content,ap);
	va_end ( ap ) ;
}

void log_warn(const char* file,const char* line,const char* content , ...){
	va_list ap ;
	va_start ( ap , content ) ;
	log_print(L_WARN,file,line,content,ap);
	va_end ( ap ) ;
}

void log_error(const char* file,const char* line,const char* content , ...){
	va_list ap ;
	va_start ( ap , content ) ;
	log_print(L_ERROR,file,line,content,ap);
	va_end ( ap ) ;
}
