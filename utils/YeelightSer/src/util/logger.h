/*
 * logger.h
 *
 *  Created on: 2015年5月5日
 *      Author: yeso
 */

#ifndef UTIL_LOGGER_H_
#define UTIL_LOGGER_H_

#include <stdarg.h>


enum LogLevel
{
	L_TRACE,
	L_DEBUG,
	L_INFO,
	L_WARN,
	L_ERROR,
};

void log_set_output(const char* dest);

void log_set_level(int level);

//#define LOG_TRACE(content,args...) log_trace(__FILE__,__LINE__,content,args)
//#define LOG_TRACE(content,args...) log_trace(__FILE__,__LINE__,content,args)



void log_trace(const char* file,const char* line,const char* content , ...);

void log_debug(const char* file,const char* line,const char* content , ...);

void log_info(const char* file,const char* line,const char* content , ...);

void log_warn(const char* file,const char* line,const char* content , ...);

void log_error(const char* file,const char* line,const char* content , ...);

#define LOG_TRACE(content,args...) log_trace(__FILE__,__LINE__,content,##args)
#define LOG_DEBUG(content,args...) log_debug(__FILE__,__LINE__,content,##args)
#define LOG_ERROR(content,args...) log_error(__FILE__,__LINE__,content,##args)
#define LOG_INFO(content,args...) log_info(__FILE__,__LINE__,content,##args)
#define LOG_WARN(content,args...) log_warn(__FILE__,__LINE__,content,##args)

#endif /* UTIL_LOGGER_H_ */
