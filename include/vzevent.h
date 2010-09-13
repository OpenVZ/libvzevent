/*
 * Copyright (c) 2003-2010 by Parallels
 * All rights reserved.
 *
 */
#ifndef _VZEVENTS_H_
#define _VZEVENTS_H_

#define EVT_MAX_MESSAGE_SIZE		4000

enum {
	VZEVENT_VZCTL_EVENT_TYPE = 1,
};


typedef int sock_t;
typedef char *str_t;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef struct vzevt_handle_s {
	sock_t sock;
	str_t sock_name;
} vzevt_handle_t;

typedef struct vzevt_s {
	u32 type;
	u32 size;
	u8 buffer[1];
} vzevt_t;

#ifdef __cplusplus
extern "C" {
#endif

/* Register event listener
 */
int vzevt_register(vzevt_handle_t **h);
/* Unregister event listener
 */
void vzevt_unregister(vzevt_handle_t *h);

vzevt_t* vzevt_alloc(u32 type, u32 size, void *buf);
void vzevt_free(vzevt_t* evt);

/* send event to all registered by vzevt_register()
 */
int vzevt_send(vzevt_handle_t *h, u32 type, u32 size, void *buf);
/* Recive event message
 * @return      0 - no message
 *              1 - message received
 *              -1- error
 */
int vzevt_recv(vzevt_handle_t *h, vzevt_t **e);
/* Recive event message, wait for ms if no event awailable
 * @return      0 - no message
 *              1 - message received
 *              -1- error
 */
int vzevt_recv_wait(vzevt_handle_t *h, unsigned int ms, vzevt_t **e);
/* get last error message */
const char *vzevt_get_last_error();
#ifdef __cplusplus
}
#endif

#endif
