/*
 * Copyright (c) 2003-2010 by Parallels
 * All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "vzevent.h"
#include "vzevent_error.h"

#define MAX_PATH	1024

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX	108
#endif


static const char *g_evt_dir = "/var/run/vzevents";

static char *gen_unx_sockname(void);

static int create_evt_dir(void)
{
	int res;
	struct stat st;
	mode_t mode = S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH;

	if (stat(g_evt_dir, &st) == 0)
		return 0;
	res = mkdir(g_evt_dir, mode);
	if (res)
		vzevt_err(VZEVT_ERR_FATAL, "mkdir: %s", strerror(errno));

	return res;
}

static void close_sock(vzevt_handle_t *h)
{
	char path[MAX_PATH];

	close(h->sock);
	if (h->sock_name) {
		snprintf(path, sizeof(path), "%s/%s",
				g_evt_dir, h->sock_name);
		unlink(path);
		free(h->sock_name);
		h->sock_name = NULL;
	}
}

static int open_sock(const char *sname)
{
	struct sockaddr_un addr;
	int s, res;

	if ((strlen(sname) + strlen(g_evt_dir) + 1) >= UNIX_PATH_MAX)
		return vzevt_err(VZEVT_ERR_FATAL, "Invalid sun_path: %s/%s",
				g_evt_dir, sname);

	res = create_evt_dir();
	if (res)
		return vzevt_err(VZEVT_ERR_FATAL,
				"Can't create directory %s: %s", g_evt_dir,
				strerror(errno));

	s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (s == -1)
		return vzevt_err(VZEVT_ERR_FATAL,
				"can't create socket: %s",
				strerror(errno));

	addr.sun_family = PF_UNIX;
	snprintf(addr.sun_path, UNIX_PATH_MAX, "%s/%s", g_evt_dir, sname);
	res = bind(s, (struct sockaddr*)&addr, SUN_LEN(&addr));
	if (res == -1) {
		if (errno == EADDRINUSE) {
			unlink(sname);
			res = bind(s, (struct sockaddr *)&addr, SUN_LEN(&addr));
		}
	}

	if (res == 0) {
		res = fcntl(s, F_SETFL, O_NONBLOCK);
		if (res == -1) {
			vzevt_err(VZEVT_ERR_FATAL,
					"Can't set socket to non-blocked mode: %s",
					strerror(errno));
			close(s); s = -1;
		}
	} else {
		vzevt_err(VZEVT_ERR_FATAL,
			"Can't create/open socket to read events: %s",
				strerror(errno));
		close(s); s = -1;
	}

	return s;
}

int vzevt_register(vzevt_handle_t **h)
{
	vzevt_handle_t *res_h;

	/* allocate handle */
	res_h = (vzevt_handle_t *)malloc(sizeof(vzevt_handle_t));
	if (!res_h)
		return vzevt_err(VZEVT_ERR_NOMEM, "nomem");

	bzero(res_h, sizeof(vzevt_handle_t));

	/* generate socket name ... */
	res_h->sock_name = gen_unx_sockname();
	if (!res_h->sock_name) {
		free(res_h);
		return vzevt_err(VZEVT_ERR_NOMEM, "nomem");
	}

	res_h->sock = open_sock(res_h->sock_name);
	if (res_h->sock == -1) {
		close_sock(res_h);
		free(res_h);
		return -1;
	}

	*h = res_h;
	return VZEVT_ERR_OK;
}

void vzevt_unregister(vzevt_handle_t *h)
{
	if (h) {
	    close_sock(h);
	    free(h);
	}
}

static int vzevt_send_evt(vzevt_handle_t *h, vzevt_t *evt)
{
	DIR *dh;
	struct stat st;
	struct dirent *dp;
	struct sockaddr_un addr;
	int sock;

	if (!evt || (evt->size > EVT_MAX_MESSAGE_SIZE))
		return vzevt_err(VZEVT_ERR_INVAL,
				"evt_send: incorrect event size: %d",
				evt->size);
	// no listeners
	if (stat(g_evt_dir, &st) != 0)
		return 0;
	/* get registered unix sockets list from g_evt_dir directory... */
	dh = opendir(g_evt_dir);
	if (!dh)
		return vzevt_err(VZEVT_ERR_FATAL,
			"can't open directory %s", g_evt_dir);

	// Create socket
	if (h == NULL) {
		sock = socket(PF_UNIX, SOCK_DGRAM, 0);
		if (sock == -1)
			return vzevt_err(VZEVT_ERR_FATAL,
				"Can't create/open socket for send event: %s",
				strerror(errno));
	} else {
		sock = h->sock;
	}

	addr.sun_family = PF_UNIX;

	while ((dp = readdir(dh))) {
		if (strcmp(dp->d_name, "..") == 0 ||
		    strcmp(dp->d_name, ".") == 0)
				continue;
		if (h != NULL && strcmp(dp->d_name, h->sock_name) == 0)
			continue;

		snprintf(addr.sun_path,UNIX_PATH_MAX, "%s/%s",
			 g_evt_dir, dp->d_name);
		if (stat(addr.sun_path, &st) != -1 &&
			S_ISSOCK(st.st_mode))
		{
			if (sendto(sock, evt, sizeof(vzevt_t) + evt->size, 0,
				(struct sockaddr *)&addr, SUN_LEN(&addr)) == -1)
			{
				if (errno == ECONNREFUSED)
					// listener is dead
					unlink(addr.sun_path);
				else
					 vzevt_err(VZEVT_ERR_FATAL, "Can not send the event to '%s': %s",
						dp->d_name, strerror(errno));
			}
		}
	}

	if (h == NULL)
		close(sock);

	closedir(dh);
	return VZEVT_ERR_OK;
}

void vzevt_free(vzevt_t *evt)
{
	free(evt);
}

vzevt_t* vzevt_alloc(u32 type, u32 size, void *buf)
{
	vzevt_t *e = (vzevt_t *)malloc(size + sizeof(vzevt_t));
	if (e == NULL)
		return e;

	e->type = type;
	e->size = size;
	memcpy(e->buffer, buf, size);
	return e;
}

int vzevt_send(vzevt_handle_t *h, u32 type, u32 size, void *buf)
{
	int res;
	vzevt_t *evt;

	evt = vzevt_alloc(type, size, buf);
	if (evt == NULL)
		return vzevt_err(VZEVT_ERR_NOMEM, "nomem");

	res = vzevt_send_evt(h, evt);
	vzevt_free(evt);

	return res;
}

/* Recive event message
 * @return	0 - no message
 *		1 - message recived
 *		-1- error
 */
int vzevt_recv(vzevt_handle_t *h /* in */, vzevt_t **evt /* out */)
{
	vzevt_t hdr, *res = NULL;
	struct sockaddr from;
	socklen_t fromlen = 0;
	int rb;

	if (!h || !evt)
		return vzevt_err(VZEVT_ERR_INVAL, "evt_recv: invalid arg");

	rb = recvfrom(h->sock, &hdr, sizeof(vzevt_t), MSG_PEEK, &from, &fromlen);
	if (rb == sizeof(vzevt_t)) {
		int sz;

		sz = sizeof(vzevt_t) + hdr.size;

		res = (vzevt_t *)malloc(sz);
		if (!res)
	                return vzevt_err(VZEVT_ERR_NOMEM, "evt_recv: nomem");

		rb = recvfrom(h->sock, res, sz, 0, &from, &fromlen);
		if (rb == sz) {
			*evt = res;
			return 1;
		} else {
			free(res);
		}
	} else if (rb > 0) {
		rb = 0; /* no received events */
	}

	if (rb == -1) {
		if (errno == EAGAIN)
			rb = 0; /* no received events */
		else
			vzevt_err(VZEVT_ERR_FATAL, "vzevt_recv: %s",
				strerror(errno));
	}
	return rb;
}

int vzevt_recv_wait(vzevt_handle_t *h /* in */, unsigned int ms /* in */, vzevt_t **evt /* out */)
{
	int res;
	fd_set rfds;
	struct timeval tv;

	tv.tv_sec = ms / 1000;
	tv.tv_usec = ((time_t)(ms % 1000)) * 1000;
        FD_ZERO(&rfds);
        FD_SET(h->sock, &rfds);

	res = select(h->sock + 1, &rfds, NULL, NULL, &tv);
	if (res == -1) {
		if (errno == EINTR)
			return 0;
		return vzevt_err(-1, "waiting on select failed with error %s",
			strerror(errno));
	}

	if (res > 0 && FD_ISSET(h->sock, &rfds))
		return vzevt_recv(h, evt);

	return 0;
}


/* ======================================================================= */

static char *gen_unx_sockname(void)
{
	static unsigned char call_srand = 1;
	struct timeval tv;
	size_t buflen = 12 * 3;

	char *res = (char *)malloc(buflen);
	if (res == NULL) {
		vzevt_err(VZEVT_ERR_NOMEM, "gen_unx_sockname: nomem");
		return NULL;
	}

	gettimeofday(&tv,NULL);
	if (call_srand) {
		time_t now = time(NULL);
		call_srand = 0;
		srand(now);
	}
	snprintf(res, buflen, "%x-%x-%d",
			(unsigned int)tv.tv_sec,
			(unsigned int)tv.tv_usec,
			rand());
	return res;
}

