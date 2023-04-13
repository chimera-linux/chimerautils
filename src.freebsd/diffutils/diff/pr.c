/*-
 * Copyright (c) 2017 Baptiste Daroussin <bapt@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/wait.h>

#include <err.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#include "pr.h"
#include "diff.h"
#include "xmalloc.h"

#define _PATH_PR "/usr/bin/pr"

static int sigpipe[2] = {-1, -1};
static struct pollfd poll_fd;

static void
handle_sig(int signo)
{
	write(sigpipe[1], &signo, sizeof(signo));
}

struct pr *
start_pr(char *file1, char *file2)
{
	int pfd[2];
	pid_t pid;
	char *header;
	struct pr *pr;

	pr = xcalloc(1, sizeof(*pr));

	xasprintf(&header, "%s %s %s", diffargs, file1, file2);
	signal(SIGPIPE, SIG_IGN);
	fflush(stdout);
	rewind(stdout);
	if (pipe(pfd) == -1)
		err(2, "pipe");
	if (sigpipe[0] < 0) {
		if (pipe(sigpipe) == -1)
			err(2, "pipe");
		if (fcntl(sigpipe[0], F_SETFD, FD_CLOEXEC) == -1)
			err(2, "fcntl");
		if (fcntl(sigpipe[1], F_SETFD, FD_CLOEXEC) == -1)
			err(2, "fcntl");
		if (signal(SIGCHLD, handle_sig) == SIG_ERR)
			err(2, "signal");
		poll_fd.fd = sigpipe[0];
		poll_fd.events = POLLIN;
	}
	poll_fd.revents = 0;
	switch ((pid = fork())) {
	case -1:
		status |= 2;
		free(header);
		err(2, "No more processes");
	case 0:
		/* child */
		if (pfd[0] != STDIN_FILENO) {
			dup2(pfd[0], STDIN_FILENO);
			close(pfd[0]);
		}
		close(pfd[1]);
		execl(_PATH_PR, _PATH_PR, "-h", header, (char *)0);
		_exit(127);
	default:

		/* parent */
		if (pfd[1] != STDOUT_FILENO) {
			pr->ostdout = dup(STDOUT_FILENO);
			dup2(pfd[1], STDOUT_FILENO);
			close(pfd[1]);
		}
		close(pfd[0]);
		rewind(stdout);
		free(header);
		pr->cpid = pid;
	}
	return (pr);
}

/* close the pipe to pr and restore stdout */
void
stop_pr(struct pr *pr)
{
	int wstatus;
	int done = 0;

	if (pr == NULL)
		return;

	fflush(stdout);
	if (pr->ostdout != STDOUT_FILENO) {
		close(STDOUT_FILENO);
		dup2(pr->ostdout, STDOUT_FILENO);
		close(pr->ostdout);
	}
	while (!done) {
		pid_t wpid;
		int npe = poll(&poll_fd, 1, -1);
		if (npe == -1) {
			if (errno == EINTR) continue;
			err(2, "poll");
		}
		if (poll_fd.revents != POLLIN)
			continue;
		if (read(poll_fd.fd, &npe, sizeof(npe)) < 0)
			err(2, "read");
		while ((wpid = waitpid(-1, &wstatus, WNOHANG)) > 0) {
			if (wpid != pr->cpid) continue;
			if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) != 0)
				errx(2, "pr exited abnormally");
			else if (WIFSIGNALED(wstatus))
				errx(2, "pr killed by signal %d",
				    WTERMSIG(wstatus));
			done = 1;
			break;
		}
	}
	free(pr);
}
