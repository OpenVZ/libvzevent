/*
 * Copyright (c) 2003-2010 by Parallels
 * All rights reserved.
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

