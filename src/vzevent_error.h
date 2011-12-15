/*
 * Copyright (c) 2003-2010 by Parallels
 * All rights reserved.
 *
 */
#ifndef _VZEVT_ERROR_H_
#define _VZEVT_ERROR_H_

#define VZEVT_ERR_OK      0
#define VZEVT_ERR_INVAL   1
#define VZEVT_ERR_FATAL   2
#define VZEVT_ERR_NOMEM   3

int vzevt_err(int err_code, const char *fmt, ...);

#endif
