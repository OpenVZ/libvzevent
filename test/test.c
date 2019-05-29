/*
 * Copyright (c) 2015-2017, Parallels International GmbH
 * Copyright (c) 2017-2019 Virtuozzo International GmbH. All rights reserved.
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
 * Our contact details: Virtuozzo International GmbH, Vordergasse 59, 8200
 * Schaffhausen, Switzerland.
 */

#include <stdlib.h>
#include <stdio.h>

#include "vzevent.h"

int test_reciver()
{
	int ret;
	vzevt_handle_t *h;
	vzevt_t *evt;

	ret = vzevt_register(&h);
	if (ret) {
		printf("evt_register: %s\n",
			vzevt_get_last_error());
		return -1;
	}
	ret = vzevt_recv(h, &evt);
	printf("vzevt_recv: ret=%d\n", ret);
	do {
		ret = vzevt_recv_wait(h, 10000, &evt);
		if (ret > 0) {
			printf("evt_recv_wait: type=%d size=%d data%d\n",
				evt->type, evt->size, *(int*)evt->buffer);
			vzevt_free(evt);
		}
		if (ret < 0) {
			printf("failed vzevt_recv_wait: %s\n",
				vzevt_get_last_error());
			break;
		} else
			printf("vzevt_recv_wait ret=0\n");

	} while (ret != -1);

	vzevt_unregister(h);
	return 0;
}

int test_sender()
{
	int i, ret;

	for (i = 0; i < 10; i++) {
		ret = vzevt_send(NULL, 0, sizeof(i), &i);
		if (ret) {
			printf("failed evt_send: %s\n",
					vzevt_get_last_error());
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("args\n");
		return -1;
	}

	if (argv[1][0] == 'r')
		test_reciver();
	else
		test_sender();

	return 0;
}
