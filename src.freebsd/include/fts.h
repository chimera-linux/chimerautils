/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993
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
 *
 *	@(#)fts.h	8.3 (Berkeley) 8/14/94
 */

#ifndef	_FTS_H_
#define	_FTS_H_

#ifdef _CHIMERAUTILS_BUILD
#include "config-compat.h"
#endif
 
#if !defined(_CHIMERAUTILS_BUILD) || !defined(HAVE_FTS_OPEN)

#include <sys/types.h>

typedef struct {
	struct _ftsent *fts_cur;	/* current node */
	struct _ftsent *fts_child;	/* linked list of children */
	struct _ftsent **fts_array;	/* sort array */
	dev_t fts_dev;		/* starting device # */
	char *fts_path;			/* path for this descent */
	int fts_rfd;			/* fd for root */
	size_t fts_pathlen;		/* sizeof(path) */
	size_t fts_nitems;		/* elements in the sort array */
	int (*fts_compar)		/* compare function */
	    (const struct _ftsent **, const struct _ftsent **);

/* valid for fts_open() */
#define	FTS_COMFOLLOW	0x000001	/* follow command line symlinks */
#define	FTS_LOGICAL	0x000002	/* logical walk */
#define	FTS_NOCHDIR	0x000004	/* don't change directories */
#define	FTS_NOSTAT	0x000008	/* don't get stat info */
#define	FTS_PHYSICAL	0x000010	/* physical walk */
#define	FTS_SEEDOT	0x000020	/* return dot and dot-dot */
#define	FTS_XDEV	0x000040	/* don't cross devices */
#if 0
#define	FTS_WHITEOUT	0x000080	/* return whiteout information */
#endif
#define	FTS_OPTIONMASK	0x0000ff	/* valid user option mask */

/* valid only for fts_children() */
#define	FTS_NAMEONLY	0x000100	/* child names only */

/* internal use only */
#define	FTS_STOP	0x010000	/* unrecoverable error */
	int fts_options;		/* fts_open options, global flags */
	void *fts_clientptr;		/* thunk for sort function */
} FTS;

typedef struct _ftsent {
	struct _ftsent *fts_cycle;	/* cycle node */
	struct _ftsent *fts_parent;	/* parent directory */
	struct _ftsent *fts_link;	/* next file in directory */
	long long fts_number;		/* local numeric value */
#define	fts_bignum	fts_number	/* XXX non-std, should go away */
	void *fts_pointer;		/* local address value */
	char *fts_accpath;		/* access path */
	char *fts_path;			/* root path */
	int fts_errno;			/* errno for this node */
	int fts_symfd;			/* fd for symlink */
	size_t fts_pathlen;		/* strlen(fts_path) */
	size_t fts_namelen;		/* strlen(fts_name) */

	ino_t fts_ino;		/* inode */
	dev_t fts_dev;		/* device */
	nlink_t fts_nlink;		/* link count */

#define	FTS_ROOTPARENTLEVEL	-1
#define	FTS_ROOTLEVEL		 0
	long fts_level;			/* depth (-1 to N) */

#define	FTS_D		 1		/* preorder directory */
#define	FTS_DC		 2		/* directory that causes cycles */
#define	FTS_DEFAULT	 3		/* none of the above */
#define	FTS_DNR		 4		/* unreadable directory */
#define	FTS_DOT		 5		/* dot or dot-dot */
#define	FTS_DP		 6		/* postorder directory */
#define	FTS_ERR		 7		/* error; errno is set */
#define	FTS_F		 8		/* regular file */
#define	FTS_INIT	 9		/* initialized only */
#define	FTS_NS		10		/* stat(2) failed */
#define	FTS_NSOK	11		/* no stat(2) requested */
#define	FTS_SL		12		/* symbolic link */
#define	FTS_SLNONE	13		/* symbolic link without target */
#if 0
#define	FTS_W		14		/* whiteout object */
#endif
	int fts_info;			/* user status for FTSENT structure */

#define	FTS_DONTCHDIR	 0x01		/* don't chdir .. to the parent */
#define	FTS_SYMFOLLOW	 0x02		/* followed a symlink to get here */
#if 0
#define	FTS_ISW		 0x04		/* this is a whiteout object */
#endif
	unsigned fts_flags;		/* private flags for FTSENT structure */

#define	FTS_AGAIN	 1		/* read node again */
#define	FTS_FOLLOW	 2		/* follow symbolic link */
#define	FTS_NOINSTR	 3		/* no instructions */
#define	FTS_SKIP	 4		/* discard node */
	int fts_instr;			/* fts_set() instructions */

	struct stat *fts_statp;		/* stat(2) information */
	char *fts_name;			/* file name */
	FTS *fts_fts;			/* back pointer to main FTS */
} FTSENT;

#ifdef __cplusplus
extern "C" {
#endif

FTSENT	*fts_children(FTS *, int);
int	 fts_close(FTS *);
void	*fts_get_clientptr(FTS *);
#define	 fts_get_clientptr(fts)	((fts)->fts_clientptr)
FTS	*fts_get_stream(FTSENT *);
#define	 fts_get_stream(ftsent)	((ftsent)->fts_fts)
FTS	*fts_open(char * const *, int,
	    int (*)(const FTSENT **, const FTSENT **));
FTSENT	*fts_read(FTS *);
int	 fts_set(FTS *, FTSENT *, int);
void	 fts_set_clientptr(FTS *, void *);

#ifdef __cplusplus
}
#endif

#else
#  include_next <fts.h>
#endif

#endif /* !_FTS_H_ */
