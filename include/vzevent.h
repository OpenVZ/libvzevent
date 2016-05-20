/*
 * Copyright (c) 2003-2015 Parallels IP Holdings GmbH
 *
 * This file is part of OpenVZ libraries. OpenVZ is free software; you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation; either version
 * 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/> or write to Free Software Foundation,
 * 51 Franklin Street, Fifth Floor Boston, MA 02110, USA.
 *
 * Our contact details: Parallels IP Holdings GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 *
 */
#ifndef _VZEVENTS_H_
#define _VZEVENTS_H_

#include <sys/types.h>

#define EVT_MAX_MESSAGE_SIZE		4000

enum {
	VZEVENT_VZCTL_EVENT_TYPE = 1,
};


typedef int sock_t;
typedef char *str_t;
typedef u_char u8;
typedef u_int32_t u32;

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
