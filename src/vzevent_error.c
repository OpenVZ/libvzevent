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

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>

struct last_error_info {
        int code;
        char msg[1024];
};

#ifdef __i386__
#include <pthread.h>
/* Workaround for non NPTL glibc
 * Private thread specific data */
static pthread_key_t last_err_local_storage_key;
static pthread_once_t last_err_local_storage_key_once = PTHREAD_ONCE_INIT;

static void buffer_destroy(void *buf)
{
	free(buf);
}

static void buffer_key_alloc(void)
{
	pthread_key_create(&last_err_local_storage_key, buffer_destroy);
	pthread_setspecific(last_err_local_storage_key,
			calloc(1, sizeof(struct last_error_info)));
}

static struct last_error_info *get_last_error_local_storage()
{
	pthread_once(&last_err_local_storage_key_once, buffer_key_alloc);
	return pthread_getspecific(last_err_local_storage_key);
}
#else

static __thread struct last_error_info last_err_local_storage;

static struct last_error_info *get_last_error_local_storage()
{
	return &last_err_local_storage;
}
#endif

static struct last_error_info *get_last_error_storage()
{
	return get_last_error_local_storage();
}

int vzevt_err(int err_code, const char *fmt, ...)
{
	va_list ap;
	struct last_error_info *ei;

	ei = get_last_error_storage();
	if (!ei)
		return err_code;

	ei->code = err_code;
	if (fmt == NULL || *fmt == '\0') {
		ei->msg[0] = '\0';
		return err_code;
	}

	va_start(ap, fmt);
	vsnprintf(ei->msg, sizeof(ei->msg)-1, fmt, ap);
	va_end(ap);

	fprintf(stderr, "%s\n", ei->msg);

	return err_code;
}

const char *vzevt_get_last_error()
{
	struct last_error_info *ei;

	ei = get_last_error_storage();
	if (!ei)
		return "";

	return ei->msg;
}

