/*
 * serial.h
 *
 *  Created on: 2015年5月3日
 *      Author: yeso
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <event2/event.h>
#include <event2/bufferevent.h>

typedef void (*serial_read_cb) (struct bufferevent*,const char*);
typedef void (*serial_close_cb) (void);
void serial_set_event(
		struct event_base *base,
		const char* device,
		int serial_open_mode,
		serial_read_cb cb,
		serial_close_cb sccb,
		struct bufferevent **bev);

#endif /* SERIAL_H_ */
