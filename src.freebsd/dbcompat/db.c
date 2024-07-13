/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)db.c	8.4 (Berkeley) 2/21/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>

#include <db.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_EXLOCK
#define O_EXLOCK 0
#endif
#ifndef O_SHLOCK
#define O_SHLOCK 0
#endif

DB *
dbopen(const char *fname, int flags, int mode, DBTYPE type, const void *openinfo)
{

#define	DB_FLAGS	(DB_LOCK | DB_SHMEM | DB_TXN)
#define	USE_OPEN_FLAGS							\
	(O_CREAT | O_EXCL | O_EXLOCK | O_NOFOLLOW | O_NONBLOCK | 	\
	 O_RDONLY | O_RDWR | O_SHLOCK | O_SYNC | O_TRUNC | O_CLOEXEC)

	if ((flags & ~(USE_OPEN_FLAGS | DB_FLAGS)) == 0)
		switch (type) {
		case DB_BTREE:
			return (__bt_open(fname, flags & USE_OPEN_FLAGS,
			    mode, openinfo, flags & DB_FLAGS));
		case DB_HASH:
#if 0
/* libdbcompat: not supported */
			return (__hash_open(fname, flags & USE_OPEN_FLAGS,
			    mode, openinfo, flags & DB_FLAGS));
#else
			break;
#endif
		case DB_RECNO:
			return (__rec_open(fname, flags & USE_OPEN_FLAGS,
			    mode, openinfo, flags & DB_FLAGS));
		}
	errno = EINVAL;
	return (NULL);
}

static int
__dberr_del(const struct __db *db, const DBT *dbt, u_int flags)
{
	(void)db;
	(void)dbt;
	(void)flags;
	return (RET_ERROR);
}

static int
__dberr_fd(const struct __db *db)
{
	(void)db;
	return (RET_ERROR);
}

static int
__dberr_get(const struct __db *db, const DBT *key, DBT *data, u_int flags)
{
	(void)db;
	(void)key;
	(void)data;
	(void)flags;
	return (RET_ERROR);
}

static int
__dberr_put(const struct __db *db, DBT *key, const DBT *data, u_int flags)
{
	(void)db;
	(void)key;
	(void)data;
	(void)flags;
	return (RET_ERROR);
}

static int
__dberr_seq(const struct __db *db, DBT *key, DBT *data, u_int flags)
{
	(void)db;
	(void)key;
	(void)data;
	(void)flags;
	return (RET_ERROR);
}

static int
__dberr_sync(const struct __db *db, u_int flags)
{
	(void)db;
	(void)flags;
	return (RET_ERROR);
}

/*
 * __DBPANIC -- Stop.
 *
 * Parameters:
 *	dbp:	pointer to the DB structure.
 */
void
__dbpanic(DB *dbp)
{
	/* The only thing that can succeed is a close. */
	dbp->del = __dberr_del;
	dbp->fd = __dberr_fd;
	dbp->get = __dberr_get;
	dbp->put = __dberr_put;
	dbp->seq = __dberr_seq;
	dbp->sync = __dberr_sync;
}
