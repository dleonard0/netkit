/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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

/*
 * From: @(#)announce.c	5.9 (Berkeley) 2/26/91
 */
char ann_rcsid[] = 
  "$Id: announce.c,v 1.8 2000/07/22 15:27:39 dholland Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <paths.h>
#include "prot_talkd.h"
#include "proto.h"

/*
 * Announce an invitation to talk.
 *
 * Because the tty driver insists on attaching a terminal-less
 * process to any terminal that it writes on, we must fork a child
 * to protect ourselves.
 *
 * That hasn't been true since, hrm, 4.2BSD maybe? Even in System V,
 * we can use O_NOCTTY. On the other hand, forking is a good idea in
 * case the tty hangs. But at present we don't take advantage of that;
 * we'd wait() forever.
 */


/*
 * Reject control codes and other crap.
 * FUTURE: don't clear high ascii if the tty is in 8-bit mode
 * Note that we don't need to let through tabs or newlines here.
 */
static int safechar(int ch) {
    if (ch>127 || ch<32) ch = '?';
    return ch;
}

#define max(a,b) ( (a) > (b) ? (a) : (b) )
#define N_LINES 5
#define N_CHARS 120

/*
 * Build a block of characters containing the message. 
 * It is sent blank filled and in a single block to
 * try to keep the message in one piece if the recipient
 * is in, say, vi at the time.
 */
static void
print_mesg(int fd, CTL_MSG *request, const char *remote_machine)
{
	struct timeval clocc;
	struct timezone zone;
	struct tm *localclock;
	char line_buf[N_LINES][N_CHARS];
	int sizes[N_LINES];
	char big_buf[N_LINES*(N_CHARS+2)+16];
	char *bptr, *lptr;
	int i, j, max_size;
	time_t footime;

	i = 0;
	max_size = 0;
	gettimeofday(&clocc, &zone);
	footime = clocc.tv_sec;
	localclock = localtime(&footime);
	snprintf(line_buf[i], N_CHARS, " ");
	sizes[i] = strlen(line_buf[i]);
	max_size = max(max_size, sizes[i]);
	i++;
	snprintf(line_buf[i], N_CHARS, 
		 "Message from Talk_Daemon@%s at %d:%02d ...",
		 ourhostname, localclock->tm_hour, localclock->tm_min);
	sizes[i] = strlen(line_buf[i]);
	max_size = max(max_size, sizes[i]);
	i++;
	snprintf(line_buf[i], N_CHARS, "talk: connection requested by %s@%s.",
		request->l_name, remote_machine);
	sizes[i] = strlen(line_buf[i]);
	max_size = max(max_size, sizes[i]);
	i++;
	snprintf(line_buf[i], N_CHARS, "talk: respond with:  talk %s@%s",
		 request->l_name, remote_machine);
	sizes[i] = strlen(line_buf[i]);
	max_size = max(max_size, sizes[i]);
	i++;
	snprintf(line_buf[i], N_CHARS, " ");
	sizes[i] = strlen(line_buf[i]);
	max_size = max(max_size, sizes[i]);
	i++;
	bptr = big_buf;
	*bptr++ = ''; /* send something to wake them up */
	*bptr++ = '\r';	/* add a \r in case of raw mode */
	*bptr++ = '\n';
	for (i = 0; i < N_LINES; i++) {
		/* copy the line into the big buffer */
		lptr = line_buf[i];
		while (*lptr != '\0')
			*(bptr++) = safechar(*(lptr++));
		/* pad out the rest of the lines with blanks */
		for (j = sizes[i]; j < max_size + 2; j++)
			*(bptr++) = ' ';
		*(bptr++) = '\r';	/* add a \r in case of raw mode */
		*(bptr++) = '\n';
	}
	*bptr = 0;
	write(fd, big_buf, strlen(big_buf));
}

/*
 * See if the user is accepting messages. If so, announce that 
 * a talk is requested.
 */
static int
announce_proc(CTL_MSG *request, const char *remote_machine)
{
	char full_tty[32];
	int fd;
	struct stat stbuf;

	snprintf(full_tty, sizeof(full_tty), "%s/%s", _PATH_DEV, 
		 request->r_tty);
	if (access(full_tty, F_OK) != 0)
		return FAILED;
	fd = open(full_tty, O_WRONLY|O_NOCTTY);
	if (fd<0) {
		return (PERMISSION_DENIED);
	}
	if (fstat(fd, &stbuf) < 0) {
		return (PERMISSION_DENIED);
	}
	if ((stbuf.st_mode&020) == 0) {
		return (PERMISSION_DENIED);
	}
	print_mesg(fd, request, remote_machine);
	close(fd);
	return SUCCESS;
}

int
announce(CTL_MSG *request, const char *remote_machine)
{
	int pid, val, status;

	pid = fork();
	if (pid==-1) {
		/* fork failed */
		return FAILED;
	}
	if (pid==0) {
		/* child */
		status = announce_proc(request, remote_machine);
		_exit(status);
	}

	/* we are the parent, so wait for the child */
	do {
		val = wait(&status);
		if (val == -1) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			/* shouldn't happen */
			syslog(LOG_WARNING, "announce: wait: %m");
			return (FAILED);
		}
	} while (val != pid);

	if (WIFSIGNALED(status)) {
		/* we were killed by some signal */
		return FAILED;
	}
	/* Send back the exit/return code */
	return (WEXITSTATUS(status));
}
