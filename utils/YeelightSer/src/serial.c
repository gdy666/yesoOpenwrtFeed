/*
 * serial.c

 *
 *  Created on: 2015年5月3日
 *      Author: yeso
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> //文件控制定义
#include <termios.h>//终端控制定义
#include <errno.h>

#include "serial.h"
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include "util/logger.h"

static void read_cb(struct bufferevent *bev, void *arg);
static void write_cb(struct bufferevent *bev, void *arg) {};
static void error_cb(struct bufferevent *bev, short event, void *arg);

static serial_read_cb serial_read_callback = NULL;
static serial_close_cb serial_close_callback = NULL;
//打开串口并初始化设置
static int
init_serial(const char* device , int open_mode)
{
	char dev[32];
	sprintf(dev,"/dev/%s",device);

	LOG_TRACE("open serial: %s", dev);

	int  serial_fd = open(dev, open_mode | O_NOCTTY | O_NDELAY);	//非阻塞
    if (serial_fd < 0) {
        perror("open");
        return -1;
    }

    //串口主要设置结构体termios <termios.h>
    struct termios options;

    /**1. tcgetattr函数用于获取与终端相关的参数。
    *参数fd为终端的文件描述符，返回的结果保存在termios结构体中
    */
    tcgetattr(serial_fd, &options);
    /**2. 修改所获得的参数*/
    options.c_cflag |= (CLOCAL | CREAD);//设置控制模式状态，本地连接，接收使能
    options.c_cflag &= ~CSIZE;//字符长度，设置数据位之前一定要屏掉这个位
    options.c_cflag &= ~CRTSCTS;//无硬件流控
    options.c_cflag |= CS8;//8位数据长度
    options.c_cflag &= ~CSTOPB;//1位停止位
    options.c_iflag |= IGNPAR;//无奇偶检验位
    options.c_oflag = 0; //输出模式
    options.c_lflag = 0; //不激活终端模式
    cfsetospeed(&options, B38400);//设置波特率

    /**3. 设置新属性，TCSANOW：所有改变立即生效*/
    tcflush(serial_fd, TCIFLUSH);//溢出数据可以接收，但不读
    tcsetattr(serial_fd, TCSANOW, &options);

    return serial_fd;
}

static void read_cb(struct bufferevent *bev, void *arg){
//	#define MAX_LINE    256
//    char line[MAX_LINE+1]={0};
    int n;
    evutil_socket_t fd = bufferevent_getfd(bev);

    struct evbuffer *input = bufferevent_get_input(bev);
    char * rline;
    size_t len;
    rline = evbuffer_readln(input, &len, EVBUFFER_EOL_CRLF);
    if(rline!=NULL){
    	if(serial_read_callback!=NULL){
    		serial_read_callback(bev,rline);
    	}
//    	 bufferevent_write(bev, rline, strlen(rline)); //使用buffer的方式输出结果
//    	 bufferevent_write(bev,"\n",1);
    	free(rline);
    }
}

static void error_cb(struct bufferevent *bev, short event, void *arg){
	if (event & BEV_EVENT_TIMEOUT) {
		LOG_TRACE("Timed out\n"); //if bufferevent_set_timeouts() called
	}
	else if (event & BEV_EVENT_EOF) {
		LOG_TRACE("serial closed\n");
	}
	else if (event & BEV_EVENT_ERROR) {
		LOG_TRACE("serial some other error\n");
	}
	bufferevent_free(bev);
	serial_read_callback = NULL;
	if(serial_close_callback!=NULL){
		serial_close_callback();
	}
}

void
serial_set_event(
		struct event_base *base ,
		const char* device ,
		int serial_open_mode ,
		serial_read_cb cb,
		serial_close_cb sccb,
		struct bufferevent **bev){
	int serial_fd = init_serial(device,serial_open_mode);
	*bev = bufferevent_socket_new(base, serial_fd, BEV_OPT_CLOSE_ON_FREE);

	bufferevent_setcb(*bev, read_cb, write_cb, error_cb, NULL);
	if(serial_open_mode == O_WRONLY){	//针对小米路由器环境下,只启用写
		bufferevent_enable(*bev, EV_WRITE|EV_PERSIST);
	}else{
		bufferevent_enable(*bev, EV_READ|EV_WRITE|EV_PERSIST);
	}


	serial_read_callback = cb;
	serial_close_callback = sccb;
}
