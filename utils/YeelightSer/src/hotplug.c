/*
 * hotplug.c
 *
 *  Created on: 2015年5月3日
 *      Author: yeso
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "hotplug.h"
static int hotplug_init_sock();
static void receive_hotplug_msg(evutil_socket_t hpfd, short event, void *arg);
static char* get_devFile(char* destStr, int ignoreCount);

hotplug_msg_callback hotplug_callback = NULL;
static int hotplug_init_sock(){
	const int buffersize = 1024;
	int ret;

	struct sockaddr_nl snl;
	bzero(&snl, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	int s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (s == -1){
		perror("socket");
		return -1;
	}
	setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

	ret = bind(s, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
	if (ret < 0){
		perror("bind");
		close(s);
		return -1;
	}
	return s;
}

struct event* hotplug_set_event(struct event_base *base,hotplug_msg_callback callback){
	struct event *event = NULL;
	int hpfd = hotplug_init_sock();
	evutil_make_socket_nonblocking(hpfd);
	event = event_new(base, hpfd, EV_READ|EV_PERSIST, receive_hotplug_msg, (void*)base);
	hotplug_callback = callback;
	return event;
}

static void receive_hotplug_msg(evutil_socket_t hpfd, short event, void *arg){
	char buffer[512];
	recv(hpfd, &buffer, sizeof(buffer), 0);
	//printf("%s\n", buffer);

	char action[16];
	sscanf(buffer,"%[^@]",action);

	/**
	 * 暂时先用来识别ttyUSB,有需要再增加识别其它设备
	 */
	char* devFile = NULL;
	char* str = NULL;

	str = strstr(buffer,"/tty/");
	if(str!=NULL){
		devFile = str+5;
	}

	if(devFile!=NULL){
		//printf("action:%s\tdevFile:%s\n",action,devFile);
		if(hotplug_callback!=NULL){
			hotplug_callback(action,devFile);
		}
	}

}

static char* get_devFile(char* destStr, int ignoreCount){
	int length = strlen(destStr);
	int count = 0;
	int i = 0;
	for(i=0;i<length;i++){
		if(destStr[i] == '/'){
			count++;
			if(count == ignoreCount){
				return destStr+i+1;
			}
		}
	}
	return 0;
}
