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
  "$Id: comsat.c,v 1.6 1996/08/29 22:21:05 dholland Exp $";

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

static int debug = 0;
#define	dsyslog	if (debug) syslog

#define MAXIDLE	120

static char hostname[MAXHOSTNAMELEN];
static struct utmp *utmp = NULL;
static time_t lastmsgtime;
static int nutmp, uf;

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
	if ((uf = open(_PATH_UTMP, O_RDONLY, 0)) < 0) {
		syslog(LOG_ERR, "main: %s: %m", _PATH_UTMP);
		(void) recv(0, msgbuf, sizeof(msgbuf) - 1, 0);
		exit(1);
	}
	(void)time(&lastmsgtime);
	(void)gethostname(hostname, sizeof(hostname));
	onalrm(SIGALRM);
	(void)signal(SIGALRM, onalrm);
	(void)signal(SIGTTOU, SIG_IGN);
	/* This should prevent zombies without needing to wait. */
	(void)signal(SIGCHLD, SIG_IGN);
	for (;;) {
		int cc;
		cc = recv(0, msgbuf, sizeof(msgbuf) - 1, 0);
		if (cc <= 0) {
			if (errno != EINTR)
				sleep(1);
			errno = 0;
			continue;
		}
		if (!nutmp)		/* no one has logged in yet */
			continue;
		sigblock(sigmask(SIGALRM));
		msgbuf[cc] = '\0';
		(void)time(&lastmsgtime);
		mailfor(msgbuf);
		sigsetmask(0L);
	}
}

static void onalrm(int signum) {
	static int utmpsize;		/* last malloced size for utmp */
	static int utmpmtime;		/* last modification time for utmp */
	struct stat statbf;

	if (signum!=SIGALRM) {
	    dsyslog(LOG_DEBUG, "wild signal %d\n", signum);
	    return;
	}

	if (time((time_t *)NULL) - lastmsgtime >= MAXIDLE)
		exit(0);
	(void)alarm(15);
	(void)fstat(uf, &statbf);
	if (statbf.st_mtime > utmpmtime) {
		utmpmtime = statbf.st_mtime;
		if (statbf.st_size > utmpsize) {
			utmpsize = statbf.st_size + 10 * sizeof(struct utmp);
			if (utmp)
				utmp = (struct utmp *)realloc((char *)utmp, utmpsize);
			else
				utmp = (struct utmp *)malloc(utmpsize);
			if (!utmp) {
				syslog(LOG_ERR, "malloc failed");
				exit(1);
			}
		}
		(void)lseek(uf, 0L, L_SET);
		nutmp = read(uf, utmp, (int)statbf.st_size)/sizeof(struct utmp);
	}
}

static void mailfor(char *name)
{
	struct utmp *utp = &utmp[nutmp];
	char *cp;
	off_t offset;

	cp = strchr(name, '@');
	if (!cp) return;

	*cp = '\0';
	offset = atoi(cp + 1);
	while (--utp >= utmp)
		if (!strncmp(utp->ut_name, name, sizeof(utmp[0].ut_name)))
			notify(utp, offset);
}


static void notify(struct utmp *utp, off_t offset)
{
	FILE *tp;
	struct stat stb;
	struct termios tbuf;
	char tty[20];
	char name[sizeof(utmp[0].ut_name) + 1];
	const char *cr;

	(void)snprintf(tty, sizeof(tty), "%s%.*s",
		       _PATH_DEV, (int)sizeof(utp->ut_line), utp->ut_line);
	/*
	 * Um. This will barf on any scheme involving ttys in subdirectories.
	 */
	if (strchr(tty + sizeof(_PATH_DEV) - 1, '/')) {
		/* A slash is an attempt to break security... */
		syslog(LOG_AUTH | LOG_NOTICE, "'/' in \"%s\"", tty);
		return;
	}

	if (stat(tty, &stb) || !(stb.st_mode & S_IEXEC)) {
		dsyslog(LOG_DEBUG, "%s: wrong mode on %s", utp->ut_name, tty);
		return;
	}
	dsyslog(LOG_DEBUG, "notify %s on %s\n", utp->ut_name, tty);

	if (fork()) {
		/* parent process */
		return;
	}
	/* child process */
	(void)signal(SIGALRM, SIG_DFL);
	(void)alarm((u_int)30);
	if ((tp = fopen(tty, "w")) == NULL) {
		dsyslog(LOG_ERR, "fopen of tty %s failed", tty);
		_exit(-1);
	}
	tcgetattr(fileno(tp), &tbuf);
	if ((tbuf.c_oflag & OPOST) && (tbuf.c_oflag & ONLCR)) cr = "\n";
	else cr = "\r\n";

	(void)strncpy(name, utp->ut_name, sizeof(utp->ut_name));
	name[sizeof(name) - 1] = '\0';
	(void)fprintf(tp, "%s\aNew mail for %s@%.*s\a has arrived:%s----%s",
	    cr, name, (int)sizeof(hostname), hostname, cr, cr);
	jkfprintf(tp, name, offset, cr);
	(void)fclose(tp);
	_exit(0);
}

static void jkfprintf(FILE *tp, const char *name, off_t offset, const char *cr)
{
	register char *cp, ch;
	register FILE *fi;
	register int linecnt, charcnt, inheader;
	register struct passwd *p;
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

	(void)fseek(fi, offset, L_SET);
	/*
	 * Print the first 7 lines or 560 characters of the new mail
	 * (whichever comes first).  Skip header crap other than
	 * From, Subject, To, and Date.
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
			    (strncmp(line, "From:", 5) &&
			    strncmp(line, "Subject:", 8)))
				continue;
		}
		if (linecnt <= 0 || charcnt <= 0) {
			(void)fprintf(tp, "...more...%s", cr);
			fclose(fi);
			return;
		}
		/* strip weird stuff so can't trojan horse stupid terminals */
		for (cp = line; (ch = *cp) && ch != '\n'; ++cp, --charcnt) {
			ch = toascii(ch);
			if (!isprint(ch) && !isspace(ch))
				ch |= 0x40;
			(void)fputc(ch, tp);
		}
		(void)fputs(cr, tp);
		--linecnt;
	}
	(void)fprintf(tp, "----%s\n", cr);
	fclose(fi);
}
