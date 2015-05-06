/*
 ============================================================================
 Name        : YelightSer.c
 Author      : yeso
 Version     : 0.10
 Date		 :
 Copyright   : QQ:272288813
 Description : YeeLight Xiaomi fucker demo
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include<fcntl.h>
#include <getopt.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <evutil.h>
#include <evhttp.h>

#include "hotplug.h"
#include "serial.h"
#include "util/prerun.h"
#include "util/logger.h"

#define  	TRUE 	1
#define 	FALSE 	0
#define 	TI 		5	//yeelight dongel time interval
#define 	YEE_REPORT_MODEL "S %04d,1,1,255,255,255,100,0,106\n"
#define 	YEELIGHT_COUNT 10


enum{
	M_Openwrt,	//以读写形式打开串口
	M_XiaoMi,	//以只写形式打开串口
	M_Yeelight,	//模仿yeelight USB适配器
};

char* MODESTR[] = {"Openwrt", "Xiaomi" , "Yeelight"};

char* DEMO_NAME = "YeelightSer";
struct event_base *base = NULL;
struct bufferevent *serial_bev = NULL;
struct event *timer_ev = NULL;
char devFileStr[32] = {0};
int flag = 0;	//程序结束标记
int daemon = 0;//当daemon为1时程序在后台执行


int run_mode = M_Openwrt;	//运行模式
int serial_open_mode = O_RDWR;	//默认读写,小米模式下只启用写
int yeelight_ready = 0;		 //USB yeelight是否已经接入系统
int XiaoMiRouter_ready = 0; // 模仿yeelight USb适配器模式下,标记小米路由器是否已经识别此设备为yeelight 适配器

void serial_receive_msg_cb(struct bufferevent* bev,const char* msg){
	LOG_TRACE("receive from serial:%s\n",msg);

	if(strstr(msg,"IDENTIFY")!=NULL){	//用来冒充yeelight USB适配器
		bufferevent_write(bev,"YEELIGHT\n",9);
		XiaoMiRouter_ready = 1;

	}else if( strstr(msg , "YEELIGHT") !=NULL){	//运行在openwrt模式下时,用来判断yeelight usb适配器是否已经就绪
		LOG_TRACE("yeelight is ready");
		yeelight_ready = 1;
	}
}

void serial_close_callback(){
	serial_bev = NULL;
	yeelight_ready = 0;
}

/**
 * timer to open serial
 */
void hotplug_timer_open_serial_cb(int fd,short _event,void *timer_ev){
	serial_set_event(
					base ,
					devFileStr,
					serial_open_mode ,
					serial_receive_msg_cb,
					serial_close_callback,
					&serial_bev);
	if(run_mode == M_Openwrt){	//适配yeelight的关键,插入usb后发送
		bufferevent_write(serial_bev,"IDENTIFY\n",9);
	}
}


void hotplug_msg_cb(const char* action, const char* devFile){
	if(strcmp(action,"add") == 0){
		/*
		 * 防止打开串口失败,发现新串口设备1秒后再打开
		 * found new serial
		 * add a timer to open serial
		 *目前程序的设定没有考虑多个串口的情况
		 */

		LOG_TRACE("found new serial:%s" , devFile);
			sprintf(devFileStr,"%s",devFile);

			struct timeval yee_tv;
			yee_tv.tv_sec = 1;
			yee_tv.tv_usec = 0;
			if(timer_ev == NULL){
				timer_ev = evtimer_new(base, hotplug_timer_open_serial_cb, timer_ev);
			}
			evtimer_add(timer_ev, &yee_tv);
	}

}

void yee_timer_reporter(){
	int i;
	char str[32];
	for (i=1; i <=YEELIGHT_COUNT ; i++){
		sprintf(str,YEE_REPORT_MODEL,i);
		bufferevent_write(serial_bev,str,strlen(str));
	}
}

void yee_time_cb(int fd,short _event,void *argc){
	struct event *yee_ev = *(struct event **)argc;
	struct timeval yee_tv;
	yee_tv.tv_sec = TI;
	yee_tv.tv_usec = 0;
	evtimer_add(yee_ev,&yee_tv);

	if(serial_bev == NULL){
		return;
	}
	if(XiaoMiRouter_ready == 1){
		yee_timer_reporter();
	}
}

int send_to_serial(const char* str){
	if(serial_bev!=NULL){
		bufferevent_write(serial_bev,str,strlen(str));
		return TRUE;
	}
	return FALSE;
}


void http_request_handler(struct evhttp_request *req, void *arg) {
    struct evbuffer *databuf=evbuffer_new();

    evbuffer_add_printf(databuf,"current mode: %s<br>",MODESTR[run_mode]);

   switch (run_mode) {
	case M_Openwrt:
		 if(yeelight_ready){
		    	evbuffer_add_printf(databuf, "YeelightSer : Yeelight USB is ready!");
		    }else{
		    	evbuffer_add_printf(databuf, "YeelightSer : Yeelight USB is not ready!");
		    }
		break;
	case M_XiaoMi:
		break;
	case M_Yeelight:
		if(XiaoMiRouter_ready){
			evbuffer_add_printf(databuf, "YeelightSer : XiaoMi Router is ready!");
		}else{
			evbuffer_add_printf(databuf, "YeelightSer : XiaoMi Router is not ready!");
		}
		break;
	default:
		break;
}

    evhttp_send_reply_start(req,200,"OK");
    evhttp_send_reply_chunk(req,databuf);
    evhttp_send_reply_end(req);
    evbuffer_free(databuf);
}

void http_yeelight_serial_control_handler(struct evhttp_request *req, void *arg) {
    struct evbuffer *databuf=evbuffer_new();

    evbuffer_add_printf(databuf,"current mode:%s\n",MODESTR[run_mode]);

    char *uri = evhttp_request_uri(req);
   // char *decoded_uri = evhttp_decode_uri(uri);
    struct evkeyvalq params;
    evhttp_parse_query(uri, &params);
    char* cmd = evhttp_find_header(&params,"cmd");
    printf("cmd:%s\n",cmd);
    if(cmd!=NULL){
    	int ret = send_to_serial(cmd);
    	send_to_serial("\n");
    	if(ret==TRUE){
    		evbuffer_add_printf(databuf,"{\"state\":\"success\"}");
    	}else{
    		evbuffer_add_printf(databuf,"{\"state\":\"serial not ready\"}");
    	}
    }else{
    	evbuffer_add_printf(databuf,"{\"state\":\"cmd error!\"}");
    }

    //free(decoded_uri);

    evhttp_send_reply_start(req,200,"OK");
    evhttp_send_reply_chunk(req,databuf);
    evhttp_send_reply_end(req);
    evbuffer_free(databuf);
}

void set_current_mode(const char* mode){
	if(strcmp(mode,"Xiaomi") == 0 ){
		run_mode = M_XiaoMi;
		serial_open_mode = O_WRONLY;
		LOG_TRACE("Run at XiaoMi Mode");
	}else if( strcmp(mode, "Yeelight") == 0){
		run_mode = M_Yeelight;
		LOG_TRACE("Run at Yeelight Mode");
	}else{
		run_mode = M_Openwrt;
		LOG_TRACE("Run at Openwrt Mode");
	}
}


void set_serial_devicestr(const char* serial_list){
	/**
	 * 防止使用"ls /dev/ttyUSB*"时候读入了多个usb串口设备,
	 * 目前设计只支持一个串口设备,因为懒得搞...额...
	 */
	if(serial_list == NULL){
		return;
	}
	sscanf(serial_list, "%s", devFileStr);
	LOG_TRACE( "set to open serial on start: %s", devFileStr);
}

static void usage(){
		fprintf(stderr, "options:\n"
				"\t-d\trun YeelightSer at background\n"
				"\t-h\tshow this help and exit\n"
				"\t-k\tkill the YeelightSer Process\n"
				"\t-mXXX set run mode ,XXX = Openwrt/Xiaomi/Yeelight\n"
				"\t-s\tset a serial which will be opened on start\n");
}


/*
 * 处理程序启动参数
 */
static void deal_arg ( uint8_t argc , char *argv[] ) {
	char opt;
		while((opt=getopt(argc, argv, "dhkm:s::")) != -1){
			switch ( opt ) {

				case 'd'://程序在后台执行
					daemon = 1;
					break;

				case 'h'://显示帮助信息
					usage();
					exit(EXIT_SUCCESS);
					break;

				case 'k'://结束服务进程
					flag = 1;
					break;
				case 'm':	//设置当前模式 ,Openwrt,XiaoMi,Yeelight
					set_current_mode(optarg);
					break;
				case 's':	//启动的时候标记要在程序启动后立刻打开的串口
					set_serial_devicestr(optarg);
					break;
				default:
					usage();
					exit(EXIT_FAILURE);
					break;
			}
		}
}


int main(int argc , char *argv[]) {
	deal_arg(argc,argv);
	if(daemon == 1){
		init_daemon(flag);
		log_set_output("/tmp/YeelightSer.log");
	}else{
		check_running(flag);
	}

	base = event_base_new();
	struct event *hotplugEvent = hotplug_set_event(base, hotplug_msg_cb);
	event_add(hotplugEvent, NULL);

	if(strlen(devFileStr) > 0){	//程序启动的时候使用了-s参数
		hotplug_timer_open_serial_cb(0,0,NULL);
	}

	if(run_mode == M_Yeelight){
			/**
			 * 冒充yeelihgt,定时反馈虚拟的60个yeelight灯泡的信息,正常情况下你会在小米APP上面发现60个灯泡..
			 */
		struct event *yee_ev = NULL;
		struct timeval yee_tv;
		yee_tv.tv_sec = TI;
		yee_tv.tv_usec = 0;
		yee_ev = evtimer_new(base,yee_time_cb,&yee_ev);
		evtimer_add(yee_ev,&yee_tv);
	}


	/**
	 * http Ser
	 */
	struct evhttp *httpd = evhttp_new(base);
	evhttp_bind_socket(httpd,"0.0.0.0",4221);
	evhttp_set_gencb(httpd, http_request_handler, NULL);
	evhttp_set_cb(httpd, "/send", http_yeelight_serial_control_handler, NULL);

	event_base_dispatch(base);

	return EXIT_SUCCESS;
}
