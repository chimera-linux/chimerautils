/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993, 1994
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

#include <sys/param.h>
#include <sys/acl.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <sys/syscall.h>
#include <acl/libacl.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sysexits.h>
#include <unistd.h>

#include "extern.h"

struct open_how {
	uint64_t flags;
	uint64_t mode;
	uint64_t resolve;
};

#define RESOLVE_NO_MAGICLINKS 0x02
#define RESOLVE_BENEATH 0x08

int openat_beneath(int dfd, const char *path, int flags, bool beneath, mode_t mode) {
	if (!beneath)
		return openat(dfd, path, flags, mode);
	struct open_how how;
	how.flags = flags;
	if (flags & (O_CREAT | O_TMPFILE))
		how.mode = mode & 07777; /* EINVAL if it contains more stuff */
	else
		how.mode = 0; /* EINVAL if nonzero */
	how.resolve = RESOLVE_BENEATH | RESOLVE_NO_MAGICLINKS;
	long fd;
	for (;;) {
		fd = syscall(SYS_openat2, dfd, path, &how, sizeof(how));
		if (fd < 0) {
			/* the documentation specifies RESOLVE_BENEATH may
			 * trigger EAGAIN as a temporary condition, try again
			 *
			 * EXDEV is the errno for RESOLVE_BENEATH violations
			 * on Linux, we want to translate for better error
			 * messages
			 *
			 * could we handle ENOSYS? probably just let it fail
			 * as we don't support older kernels anyway, we could
			 * do manual path resolution but meh
			 */
			switch (errno) {
				case EAGAIN: continue;
				case EXDEV: errno = EACCES; break;
			}
			return -1;
		}
		break;
	}
	return (int)fd;
}

static int unlinkat_beneath(int dfd, const char *path, bool beneath) {
	if (!beneath)
		return unlinkat(dfd, path, 0);
	/* code crimes because linux lol; anyway resolve to an fd first
	 * always use O_NOFOLLOW because unlinkat will delete links
	 */
	int fd = openat_beneath(dfd, path, O_PATH | O_NOFOLLOW, true, 0);
	if (fd < 0) {
		return -1;
	}
	/* fetch the file descriptor from procfs...
	 *
	 * this should resolve to an absolute path to the file for as
	 * long as the file descriptor is present and the file has not
	 * been deleted; we only use this for unlink which never follows
	 * links so this should be safe to do
	 */
	char pdesc[128], llink[PATH_MAX];
	ssize_t len;
	snprintf(pdesc, sizeof(pdesc), "/proc/self/fd/%d", fd);
	len = readlink(pdesc, llink, sizeof(llink) - 1);
	if (len < 0) {
		/* could not resolve */
		close(fd);
		return -1;
	} else if (len == 0) {
		/* file does not seem to exist anymore at that path */
		close(fd);
		return 0;
	}
	llink[len] = '\0';
	int ret = unlink(llink);
	if (ret < 0) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

#define	cp_pct(x, y)	((y == 0) ? 0 : (int)(100.0 * (x) / (y)))

/*
 * Memory strategy threshold, in pages: if physmem is larger then this, use a 
 * large buffer.
 */
#define PHYSPAGES_THRESHOLD (32*1024)

/* Maximum buffer size in bytes - do not allow it to grow larger than this. */
#define BUFSIZE_MAX (2*1024*1024)

/*
 * Small (default) buffer size in bytes. It's inefficient for this to be
 * smaller than MAXPHYS.
 */
#define BUFSIZE_SMALL (MAXPHYS)

/*
 * Prompt used in -i case.
 */
#define YESNO "(y/n [n]) "

static ssize_t
copy_fallback(int from_fd, int to_fd)
{
	static char *buf = NULL;
	static size_t bufsize;
	ssize_t rcount, wresid, wcount = 0;
	char *bufp;

	if (buf == NULL) {
		if (sysconf(_SC_PHYS_PAGES) > PHYSPAGES_THRESHOLD)
			bufsize = MIN(BUFSIZE_MAX, MAXPHYS * 8);
		else
			bufsize = BUFSIZE_SMALL;
		buf = malloc(bufsize);
		if (buf == NULL)
			err(1, "Not enough memory");
	}
	rcount = read(from_fd, buf, bufsize);
	if (rcount <= 0)
		return (rcount);
	for (bufp = buf, wresid = rcount; ; bufp += wcount, wresid -= wcount) {
		wcount = write(to_fd, bufp, wresid);
		if (wcount <= 0)
			break;
		if (wcount >= wresid)
			break;
	}
	return (wcount < 0 ? wcount : rcount);
}

int
copy_file(const FTSENT *entp, bool dne, bool beneath)
{
	struct stat sb, *fs;
	ssize_t wcount;
	off_t wtotal;
	int ch, checkch, from_fd, rval, to_fd;
	bool use_copy_file_range = true;

	fs = entp->fts_statp;
	from_fd = to_fd = -1;
	if (!lflag && !sflag) {
		if ((from_fd = open(entp->fts_path, O_RDONLY, 0)) < 0 ||
		    fstat(from_fd, &sb) != 0) {
			warn("%s", entp->fts_path);
			if (from_fd >= 0)
				(void)close(from_fd);
			return (1);
		}
		/*
		 * Check that the file hasn't been replaced with one of a
		 * different type.  This can happen if we've been asked to
		 * copy something which is actively being modified and
		 * lost the race, or if we've been asked to copy something
		 * like /proc/X/fd/Y which stat(2) reports as S_IFREG but
		 * is actually something else once you open it.
		 */
		if ((sb.st_mode & S_IFMT) != (fs->st_mode & S_IFMT)) {
			warnx("%s: File changed", entp->fts_path);
			(void)close(from_fd);
			return (1);
		}
	}

	/*
	 * If the file exists and we're interactive, verify with the user.
	 * If the file DNE, set the mode to be the from file, minus setuid
	 * bits, modified by the umask; arguably wrong, but it makes copying
	 * executables work right and it's been that way forever.  (The
	 * other choice is 666 or'ed with the execute bits on the from file
	 * modified by the umask.)
	 */
	if (!dne) {
		if (nflag) {
			if (vflag)
				printf("%s%s not overwritten\n",
				    to.base, to.path);
			rval = 1;
			goto done;
		} else if (iflag) {
			(void)fprintf(stderr, "overwrite %s%s? %s",
			    to.base, to.path, YESNO);
			checkch = ch = getchar();
			while (ch != '\n' && ch != EOF)
				ch = getchar();
			if (checkch != 'y' && checkch != 'Y') {
				(void)fprintf(stderr, "not overwritten\n");
				rval = 1;
				goto done;
			}
		}

		if (fflag) {
			/* remove existing destination file */
			(void)unlinkat_beneath(to.dir, to.path, beneath);
			dne = 1;
		}
	}

	rval = 0;

	if (lflag) {
		if (linkat(AT_FDCWD, entp->fts_path, to.dir, to.path, 0) != 0) {
			warn("%s%s", to.base, to.path);
			rval = 1;
		}
		goto done;
	}

	if (sflag) {
		if (symlinkat(entp->fts_path, to.dir, to.path) != 0) {
			warn("%s%s", to.base, to.path);
			rval = 1;
		}
		goto done;
	}

	if (!dne) {
		/* overwrite existing destination file */
		to_fd = openat_beneath(to.dir, to.path, O_WRONLY | O_TRUNC, beneath, 0);
	} else {
		/* create new destination file */
		to_fd = openat_beneath(to.dir, to.path, O_WRONLY | O_TRUNC | O_CREAT,
		    beneath, fs->st_mode & ~(S_ISUID | S_ISGID));
	}
	if (to_fd == -1) {
		warn("%s%s", to.base, to.path);
		rval = 1;
		goto done;
	}

	wtotal = 0;
	do {
		if (use_copy_file_range) {
			wcount = copy_file_range(from_fd, NULL,
			    to_fd, NULL, SSIZE_MAX, 0);
			if (wcount < 0) switch (errno) {
			case EINVAL: /* Prob a non-seekable FD */
			case EXDEV: /* Cross-FS link */
			case ENOSYS: /* Syscall not supported */
				use_copy_file_range = false;
				break;
			default:
				break;
			}
		}
		if (!use_copy_file_range) {
			wcount = copy_fallback(from_fd, to_fd);
		}
		wtotal += wcount;
		if (info) {
			info = 0;
			(void)fprintf(stderr,
			    "%s -> %s%s %3d%%\n",
			    entp->fts_path, to.base, to.path,
			    cp_pct(wtotal, fs->st_size));
		}
	} while (wcount > 0);
	if (wcount < 0) {
		warn("%s", entp->fts_path);
		rval = 1;
	}

	/*
	 * Don't remove the target even after an error.  The target might
	 * not be a regular file, or its attributes might be important,
	 * or its contents might be irreplaceable.  It would only be safe
	 * to remove it if we created it and its length is 0.
	 */
	if (pflag && setfile(fs, to_fd, beneath))
		rval = 1;
	if (pflag && preserve_fd_acls(from_fd, to_fd) != 0)
		rval = 1;
	if (aflag) preserve_fd_xattrs(from_fd, to_fd);
	if (close(to_fd)) {
		warn("%s%s", to.base, to.path);
		rval = 1;
	}

done:
	if (from_fd != -1)
		(void)close(from_fd);
	return (rval);
}

int
copy_link(const FTSENT *p, bool dne, bool beneath)
{
	ssize_t len;
	char llink[PATH_MAX];

	if (!dne && nflag) {
		if (vflag)
			printf("%s%s not overwritten\n", to.base, to.path);
		return (1);
	}
	if ((len = readlink(p->fts_path, llink, sizeof(llink) - 1)) == -1) {
		warn("readlink: %s", p->fts_path);
		return (1);
	}
	llink[len] = '\0';
	if (!dne && unlinkat_beneath(to.dir, to.path, beneath) != 0) {
		warn("unlink: %s%s", to.base, to.path);
		return (1);
	}
	if (symlinkat(llink, to.dir, to.path) != 0) {
		warn("symlink: %s", llink);
		return (1);
	}
	return (pflag ? setfile(p->fts_statp, -1, beneath) : 0);
}

int
copy_fifo(struct stat *from_stat, bool dne, bool beneath)
{
	if (!dne && nflag) {
		if (vflag)
			printf("%s%s not overwritten\n", to.base, to.path);
		return (1);
	}
	if (!dne && unlinkat_beneath(to.dir, to.path, beneath) != 0) {
		warn("unlink: %s%s", to.base, to.path);
		return (1);
	}
	if (mkfifoat(to.dir, to.path, from_stat->st_mode) != 0) {
		warn("mkfifo: %s%s", to.base, to.path);
		return (1);
	}
	return (pflag ? setfile(from_stat, -1, beneath) : 0);
}

int
copy_special(struct stat *from_stat, bool dne, bool beneath)
{
	if (!dne && nflag) {
		if (vflag)
			printf("%s%s not overwritten\n", to.base, to.path);
		return (1);
	}
	if (!dne && unlinkat_beneath(to.dir, to.path, beneath) != 0) {
		warn("unlink: %s%s", to.base, to.path);
		return (1);
	}
	if (mknodat(to.dir, to.path, from_stat->st_mode, from_stat->st_rdev) != 0) {
		warn("mknod: %s%s", to.base, to.path);
		return (1);
	}
	return (pflag ? setfile(from_stat, -1, beneath) : 0);
}

int
setfile(struct stat *fs, int fd, bool beneath)
{
	static struct timespec tspec[2];
	struct stat ts;
	int rval, gotstat, islink, fdval;

	rval = 0;
	fdval = fd != -1;
	islink = !fdval && S_ISLNK(fs->st_mode);
	fs->st_mode &= S_ISUID | S_ISGID | S_ISVTX |
	    S_IRWXU | S_IRWXG | S_IRWXO;

	if (!fdval) {
		fd = openat_beneath(to.dir, to.path, O_RDONLY | (islink ? O_NOFOLLOW : 0), beneath, 0);
		if (fd < 0) {
			warn("openat2: %s%s", to.base, to.path);
			/* any action will fail, might as well just return early */
			return 1;
		}
	}

	tspec[0] = fs->st_atim;
	tspec[1] = fs->st_mtim;
	if (futimens(fd, tspec)) {
		warn("utimensat: %s%s", to.base, to.path);
		rval = 1;
	}
	if (fstat(fd, &ts)) {
		gotstat = 0;
	} else {
		gotstat = 1;
		ts.st_mode &= S_ISUID | S_ISGID | S_ISVTX |
		    S_IRWXU | S_IRWXG | S_IRWXO;
	}
	/*
	 * Changing the ownership probably won't succeed, unless we're root
	 * or POSIX_CHOWN_RESTRICTED is not set.  Set uid/gid before setting
	 * the mode; current BSD behavior is to remove all setuid bits on
	 * chown.  If chown fails, lose setuid/setgid bits.
	 */
	if (!gotstat || fs->st_uid != ts.st_uid || fs->st_gid != ts.st_gid) {
		if (fchown(fd, fs->st_uid, fs->st_gid)) {
			if (errno != EPERM) {
				warn("chown: %s%s", to.base, to.path);
				rval = 1;
			}
			fs->st_mode &= ~(S_ISUID | S_ISGID);
		}
	}

	if (!gotstat || fs->st_mode != ts.st_mode) {
		if (islink ? 0 : fchmod(fd, fs->st_mode)) {
			warn("chmod: %s%s", to.base, to.path);
			rval = 1;
		}
	}

#if 0
	if (!Nflag && (!gotstat || fs->st_flags != ts.st_flags)) {
		if (fdval ? fchflags(fd, fs->st_flags) :
		    chflagsat(to.dir, to.path, fs->st_flags, atflags)) {
			/*
			 * NFS doesn't support chflags; ignore errors unless
			 * there's reason to believe we're losing bits.  (Note,
			 * this still won't be right if the server supports
			 * flags and we were trying to *remove* flags on a file
			 * that we copied, i.e., that we didn't create.)
			 */
			if (errno != EOPNOTSUPP || fs->st_flags != 0) {
				warn("chflags: %s%s", to.base, to.path);
				rval = 1;
			}
		}
	}
#endif

	/* we opened our own descriptor here */
	if (!fdval) close(fd);

	return (rval);
}

int
preserve_fd_acls(int source_fd, int dest_fd)
{
	acl_t acl;
	int acl_supported = 0, ret;

#if 0
	ret = fpathconf(source_fd, _PC_ACL_NFS4);
	if (ret > 0 ) {
		acl_supported = 1;
		acl_type = ACL_TYPE_NFS4;
	} else if (ret < 0 && errno != EINVAL) {
		warn("fpathconf(..., _PC_ACL_NFS4) failed for %s%s",
		    to.base, to.path); 
		return (-1);
	}
#endif
	if (acl_supported == 0) {
		ret = acl_extended_fd(source_fd);
		if (ret > 0 ) {
			acl_supported = 1;
		} else if (ret < 0 && errno != ENOTSUP) {
			warn("acl_extended_fd() failed for %s%s",
			    to.base, to.path);
			return (-1);
		}
	}
	if (acl_supported == 0)
		return (0);

	acl = acl_get_fd(source_fd);
	if (acl == NULL) {
		warn("failed to get acl entries while setting %s%s",
		    to.base, to.path);
		return (-1);
	}
	if (acl_set_fd(dest_fd, acl) < 0) {
		warn("failed to set acl entries for %s%s",
		    to.base, to.path);
		acl_free(acl);
		return (-1);
	}
	acl_free(acl);
	return (0);
}

int
preserve_dir_acls(const char *source_dir, const char *dest_dir)
{
	int source_fd = -1, dest_fd = -1, ret;

	if ((source_fd = open(source_dir, O_DIRECTORY | O_RDONLY)) < 0) {
		warn("%s: failed to copy ACLs", source_dir);
		return (-1);
	}
	dest_fd = (*dest_dir == '\0') ? to.dir :
	    openat_beneath(to.dir, dest_dir, O_DIRECTORY, true, 0);
	if (dest_fd < 0) {
		warn("%s: failed to copy ACLs to %s%s", source_dir,
		    to.base, dest_dir);
		close(source_fd);
		return (-1);
	}
	if ((ret = preserve_fd_acls(source_fd, dest_fd)) != 0) {
		/* preserve_fd_acls() already printed a message */
	}
	if (dest_fd != to.dir)
		close(dest_fd);
	close(source_fd);
	return (ret);
}

/* for now we don't really care about warnings or result,
 * we only support the quiet case for archive mode
 */
int
preserve_fd_xattrs(int source_fd, int dest_fd)
{
    ssize_t size;
    char buf[256], vbuf[128];
    char *names, *name, *nend;
    char *value = vbuf;
    int retval = 0, rerrno = 0;
    size_t vbufs = sizeof(vbuf);

    size = flistxattr(source_fd, NULL, 0);
    if (size < 0) {
        return 1;
    }

    if (size < (ssize_t)sizeof(buf)) {
        names = buf;
    } else {
        names = malloc(size + 1);
        if (!names) err(1, "Not enough memory");
    }

    size = flistxattr(source_fd, names, size);
    if (size < 0) {
        if (names != buf) free(names);
        return 1;
    }
    names[size] = '\0';
    nend = names + size;

    for (name = names; name != nend; name = strchr(name, '\0') + 1) {
        size = fgetxattr(source_fd, name, NULL, 0);
        if (size < 0) {
            retval = 1;
            rerrno = errno;
            continue;
        }
        if (size > (ssize_t)vbufs) {
            if (value == vbuf) value = NULL;
            value = realloc(value, size);
            if (!value) {
                err(1, "Not enough memory");
            }
            vbufs = size;
        }
        size = fgetxattr(source_fd, name, value, size);
        if (size < 0) {
            retval = 1;
            rerrno = errno;
            continue;
        }
        if (fsetxattr(dest_fd, name, value, size, 0)) {
            retval = 1;
            rerrno = errno;
        }
    }

    if (names != buf) free(names);
    if (value != vbuf) free(value);
    if (retval) {
        errno = rerrno;
    }
    return retval;
}

int
preserve_dir_xattrs(const char *source_dir, const char *dest_dir) {
    ssize_t size;
    char buf[256], vbuf[128];
    char *names, *name, *nend;
    char *value = vbuf;
    int retval = 0, rerrno = 0;
    size_t vbufs = sizeof(vbuf);

    size = llistxattr(source_dir, NULL, 0);
    if (size < 0) {
        return 1;
    }

    if (size < (ssize_t)sizeof(buf)) {
        names = buf;
    } else {
        names = malloc(size + 1);
        if (!names) err(1, "Not enough memory");
    }

    size = llistxattr(source_dir, names, size);
    if (size < 0) {
        if (names != buf) free(names);
        return 1;
    }
    names[size] = '\0';
    nend = names + size;

    for (name = names; name != nend; name = strchr(name, '\0') + 1) {
        size = lgetxattr(source_dir, name, NULL, 0);
        if (size < 0) {
            retval = 1;
            rerrno = errno;
            continue;
        }
        if (size > (ssize_t)vbufs) {
            if (value == vbuf) value = NULL;
            value = realloc(value, size);
            if (!value) {
                err(1, "Not enough memory");
            }
            vbufs = size;
        }
        size = lgetxattr(source_dir, name, value, size);
        if (size < 0) {
            retval = 1;
            rerrno = errno;
            continue;
        }
        if (lsetxattr(dest_dir, name, value, size, 0)) {
            retval = 1;
            rerrno = errno;
        }
    }

    if (names != buf) free(names);
    if (value != vbuf) free(value);
    if (retval) {
        errno = rerrno;
    }
    return retval;
}

void
usage(void)
{

	(void)fprintf(stderr, "%s\n%s\n%s\n",
	    "usage: cp [-R [-H | -L | -P]] [-f | -i | -n] [-alpsvxT] "
	    "source_file target_file",
	    "       cp [-R [-H | -L | -P]] [-f | -i | -n] [-alpsvx] "
	    "source_file ... "
	    "target_directory",
	    "       cp [-R [-H | -L | -P]] [-f | -i | -n] [-alpsvx] "
	    "-t target_directory "
	    "source_file ... ");
	exit(EX_USAGE);
}
