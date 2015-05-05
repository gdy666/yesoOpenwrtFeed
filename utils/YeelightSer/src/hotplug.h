/*
 * hotplug.h
 *
 *  Created on: 2015年5月3日
 *      Author: yeso
 */

#ifndef HOTPLUG_H_
#define HOTPLUG_H_

#include <event2/event.h>

typedef void (*hotplug_msg_callback)(char* action, char* devFile);

struct event* hotplug_set_event(struct event_base *base,hotplug_msg_callback);


#endif /* HOTPLUG_H_ */
