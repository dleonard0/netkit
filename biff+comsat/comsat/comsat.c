/*
 * Copyright (c) 1980 Regents of the University of California.
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

char copyright[] =
  "@(#) Copyright (c) 1980 Regents of the University of California.\n"
  "All rights reserved.\n";

/*
 * From: @(#)comsat.c	5.24 (Berkeley) 2/25/91
 */
char rcsid[] = 
  "$Id: comsat.c,v 1.10 1997/05/20 01:37:29 dholland Exp $";

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <stdio.h>
#include <utmp.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <pwd.h>
#include <paths.h>
#include <unistd.h>
#include <stdlib.h>

#include "../version.h"

static int debug = 0;
#define	dsyslog	if (debug) syslog

#define MAXIDLE	120

static char hostname[MAXHOSTNAMELEN];
static struct utmp *utmp = NULL;
static time_t lastmsgtime;
static int nutmp;

static void mailfor(char *name);
static void notify(struct utmp *utp, off_t offset);
static void jkfprintf(FILE *, const char *name, off_t offset, const char *cr);
static void onalrm(int);

int main(void) {
	char msgbuf[100];
	struct sockaddr_in from;
	size_t fromlen;

	/* verify proper invocation */
	fromlen = sizeof(from);
	if (getsockname(0, (struct sockaddr *)&from, &fromlen) < 0) {
		fprintf(stderr, "comsat: getsockname: %s.\n", strerror(errno));
		exit(1);
	}
	openlog("comsat", LOG_PID, LOG_DAEMON);
	if (chdir(_PATH_MAILDIR)) {
		syslog(LOG_ERR, "chdir: %s: %m", _PATH_MAILDIR);
		exit(1);
	}
	time(&lastmsgtime);
	gethostname(hostname, sizeof(hostname));
	signal(SIGTTOU, SIG_IGN);
	/* This should prevent zombies without needing to wait. */
	signal(SIGCHLD, SIG_IGN);
	signal(SIGALRM, onalrm);
	onalrm(SIGALRM);
	for (;;) {
		int cc;
		cc = recv(0, msgbuf, sizeof(msgbuf) - 1, 0);
		if (cc <= 0) {
			if (errno != EINTR) sleep(1);
			errno = 0;
			continue;
		}
		if (!nutmp) continue; /* no one has logged in yet */
		sigblock(sigmask(SIGALRM));
		msgbuf[cc] = 0;
		time(&lastmsgtime);
		mailfor(msgbuf);
		sigsetmask(0L);
	}
}

static void onalrm(int signum) {
	static int utmpsize;		/* last malloced size for utmp */
	static time_t utmpmtime;	/* last modification time for utmp */
	struct stat statbuf;
	struct utmp *uptr;
	int maxutmp;

	if (signum!=SIGALRM) {
	    dsyslog(LOG_DEBUG, "wild signal %d\n", signum);
	    return;
	}

	if (time(NULL) - lastmsgtime >= MAXIDLE) {
		exit(0);
	}
	alarm(15);
	if (stat(_PATH_UTMP, &statbuf) < 0) {
		dsyslog(LOG_DEBUG, "fstat of utmp failed: %m\n");
		return;
	}
	if (statbuf.st_mtime > utmpmtime) {
		utmpmtime = statbuf.st_mtime;
		if (statbuf.st_size > utmpsize) {
			utmpsize = statbuf.st_size + 10 * sizeof(struct utmp);
			if (utmp) {
				utmp = realloc(utmp, utmpsize);
			}
			else {
				utmp = malloc(utmpsize);
			}
			if (!utmp) {
				syslog(LOG_ERR, "malloc failed: %m");
				exit(1);
			}
		}
		maxutmp = utmpsize / sizeof(struct utmp);
		nutmp = 0;
		setutent();
		while ((uptr = getutent())!=NULL && nutmp < maxutmp) {
		    utmp[nutmp] = *uptr;
		    nutmp++;
		}
		endutent();
	}
}

static void mailfor(char *name)
{
	struct utmp *utp;
	char *cp;
	off_t offset;

	cp = strchr(name, '@');
	if (!cp) return;
	*cp = 0;
	offset = atoi(cp + 1);

	utp = &utmp[nutmp];
	while (--utp >= utmp) {
		if (!strncmp(utp->ut_name, name, sizeof(utmp[0].ut_name)))
			notify(utp, offset);
	}
}


static void notify(struct utmp *utp, off_t offset)
{
	FILE *tp;
	struct stat stb;
	struct termios tbuf;
	char tty[sizeof(utp->ut_line)+sizeof(_PATH_DEV)+1];
	char name[sizeof(utp->ut_name) + 1];
	char line[sizeof(utp->ut_line) + 1];
	const char *cr;

	strncpy(line, utp->ut_line, sizeof(utp->ut_line));
	line[sizeof(utp->ut_line)] = 0;

	strncpy(name, utp->ut_name, sizeof(utp->ut_name));
	name[sizeof(name) - 1] = '\0';

	snprintf(tty, sizeof(tty), "%s%s", _PATH_DEV, line);

	/*
	 * Um. This will barf on any scheme involving ttys in subdirectories.
	 */
	if (strchr(line, '/')) {
		/* A slash is an attempt to break security... */
		syslog(LOG_AUTH | LOG_NOTICE, "'/' in \"%s\"", tty);
		return;
	}

	if (stat(tty, &stb) || !(stb.st_mode & S_IEXEC)) {
		dsyslog(LOG_DEBUG, "%s: wrong mode on %s", name, tty);
		return;
	}
	dsyslog(LOG_DEBUG, "notify %s on %s\n", name, tty);

	if (fork()) {
		/* parent process */
		return;
	}
	/* child process */
	signal(SIGALRM, SIG_DFL);
	alarm(30);
	if ((tp = fopen(tty, "w")) == NULL) {
		dsyslog(LOG_ERR, "fopen of tty %s failed", tty);
		_exit(-1);
	}
	tcgetattr(fileno(tp), &tbuf);
	if ((tbuf.c_oflag & OPOST) && (tbuf.c_oflag & ONLCR)) cr = "\n";
	else cr = "\r\n";

	fprintf(tp, "%s\aNew mail for %s@%.*s\a has arrived:%s----%s",
		cr, name, (int)sizeof(hostname), hostname, cr, cr);
	jkfprintf(tp, name, offset, cr);
	fclose(tp);
	_exit(0);
}

static void jkfprintf(FILE *tp, const char *name, off_t offset, const char *cr)
{
	char *cp, ch;
	FILE *fi;
	int linecnt, charcnt, inheader;
	struct passwd *p;
	char line[BUFSIZ];

	/* Set effective uid to user in case mail drop is on nfs */
	if ((p = getpwnam(name)) == NULL) {
		/*
		 * If user is not in passwd file, assume that it's
		 * an attempt to break security...
		 */
		syslog(LOG_AUTH | LOG_NOTICE, "%s not in passwd file", name);
		return;
	} 
	setuid(p->pw_uid);

	fi = fopen(name, "r");
	if (fi == NULL)	return;

	fseek(fi, offset, L_SET);
	/*
	 * Print the first 7 lines or 560 characters of the new mail
	 * (whichever comes first).  Skip header crap other than
	 * From and Subject.
	 */
	linecnt = 7;
	charcnt = 560;
	inheader = 1;
	while (fgets(line, sizeof(line), fi) != NULL) {
		if (inheader) {
			if (line[0] == '\n') {
				inheader = 0;
				continue;
			}
			if (line[0] == ' ' || line[0] == '\t' ||
			    (strncasecmp(line, "From:", 5) &&
			    strncasecmp(line, "Subject:", 8)))
				continue;
		}
		if (linecnt <= 0 || charcnt <= 0) {
			fprintf(tp, "...more...%s", cr);
			fclose(fi);
			return;
		}
		/* strip weird stuff so can't trojan horse stupid terminals */
		for (cp = line; (ch = *cp) && ch != '\n'; ++cp, --charcnt) {
			ch = toascii(ch);
			if (!isprint(ch) && !isspace(ch))
				ch |= 0x40;
			fputc(ch, tp);
		}
		fputs(cr, tp);
		--linecnt;
	}
	fprintf(tp, "----%s", cr);
	fclose(fi);
}
