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
  "$Id: comsat.c,v 1.17 1999/10/17 23:18:47 dholland Exp $";

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
#include <paths.h>   /* used for _PATH_MAILDIR, _PATH_UTMP, etc. */
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
	char msgbuf[100];          /* network input buffer */
	struct sockaddr_in from;   /* name of socket */
	socklen_t fromlen;         /* holds sizeof(from) */

	/* 
	 * Verify proper invocation: get the address of the local socket
	 * that's supposed to be our stdin. If it's not a socket (the only
	 * likely error) assume we were inadvertently run from someone's
	 * shell and quit out.
	 */
	fromlen = sizeof(from);
	if (getsockname(0, (struct sockaddr *)&from, &fromlen) < 0) {
		fprintf(stderr, "comsat: getsockname: %s.\n", strerror(errno));
		exit(1);
	}

	/*
	 * openlog() opens a connection to the system logger for comsat.
	 * We log as "comsat", with the process id, to the "LOG_DAEMON"
	 * log channel.
	 */
	openlog("comsat", LOG_PID, LOG_DAEMON);

	/*
	 * Schlep over to the system mail spool.
	 */
	if (chdir(_PATH_MAILDIR)) {
		syslog(LOG_ERR, "chdir: %s: %m", _PATH_MAILDIR);
		exit(1);
	}

	/*
	 * Record the time of the last message received (the one that
	 * caused us to be run) so we can idle out later.
	 */
	time(&lastmsgtime);

	/*
	 * Grab our hostname for spewing to the user's screen.
	 */
	gethostname(hostname, sizeof(hostname));

	/*
	 * Make sure we won't get stopped by scribbling on the user's screen.
	 */
	signal(SIGTTOU, SIG_IGN);

	/* 
	 * This declares that we're not interested in child processes' 
	 * exit codes, so they will go away on their own without needing
	 * to be wait()'d for.
	 */
	signal(SIGCHLD, SIG_IGN);

	/*
	 * Periodically reread the utmp, starting now.
	 */
	signal(SIGALRM, onalrm);
	onalrm(SIGALRM);

	/*
	 * Loop forever handling biff packets.
	 */
	for (;;) {
		sigset_t sigset;
		int cc;

		errno = 0;
		cc = recv(0, msgbuf, sizeof(msgbuf) - 1, 0);
		if (cc <= 0) {
			if (errno != EINTR) sleep(1);
			continue;
		}
		if (!nutmp) {
			/* utmp is empty: no one is logged in */ 
			continue;
		}

		/* Make sure we don't get SIGALRM while doing mailfor() */
		sigemptyset(&sigset);
		sigaddset(&sigset, SIGALRM);
		sigprocmask(SIG_BLOCK, &sigset, NULL);

		/* null-terminate input */
		msgbuf[cc] = 0;

		/* update time stamp */
		time(&lastmsgtime);

		/* biff the user */
		mailfor(msgbuf);

		/* allow SIGALRM again */
		sigprocmask(SIG_UNBLOCK, &sigset, NULL);
	}
}

/*
 * This function is called at startup and every fifteen seconds while 
 * we're running, via timer signals.
 */
static void onalrm(int signum) {
	static int utmpsize;		/* last malloced size for utmp */
	static time_t utmpmtime;	/* last modification time for utmp */

	struct stat statbuf;            /* scratch storage for stat() */
	struct utmp *uptr;
	int maxutmp;

	/*
	 * Defensive programming (this should not happen)
	 */
	if (signum!=SIGALRM) {
	    dsyslog(LOG_DEBUG, "wild signal %d\n", signum);
	    return;
	}

	/*
	 * If we've been sitting around doing nothing, go away.
	 */
	if (time(NULL) - lastmsgtime >= MAXIDLE) {
		exit(0);
	}

	/*
	 * Come back here in another fifteen seconds.
	 */
	alarm(15);

	/*
	 * Check the modification time of the utmp file.
	 */
	if (stat(_PATH_UTMP, &statbuf) < 0) {
		dsyslog(LOG_DEBUG, "fstat of utmp failed: %m\n");
		return;
	}

	if (statbuf.st_mtime > utmpmtime) {
		/*
		 * utmp has changed since we last came here; reread it and
		 * save the modification time.
		 */
		utmpmtime = statbuf.st_mtime;

		/*
		 * If it's now bigger than the space we have for it, 
		 * get more space.
		 */
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

		/* This is how many utmp entries we can possibly get */
		maxutmp = utmpsize / sizeof(struct utmp);

		/* Rewind the utmp file */
		setutent();

		/*
		 * Read the utmp file, via libc, copying each entry and
		 * counting how many we get.
		 */
		nutmp = 0;
		while ((uptr = getutent())!=NULL && nutmp < maxutmp) {
		    utmp[nutmp] = *uptr;
		    nutmp++;
		}

		/* Now done; close the utmp file. */
		endutent();
	}
}

/*
 * We get here when we have mail to announce for the specified user.
 * "name" holds a biff packet, which comes in the form
 *          username@fileoffset
 */
static void mailfor(char *name)
{
	struct utmp *utp;
	char *cp;
	off_t offset;

	/* Break off the file offset part and convert it to an integer. */
	cp = strchr(name, '@');
	if (!cp) return;
	*cp = 0;
	offset = atol(cp + 1);

	/* Look through the utmp and call notify() for each matching login. */
	utp = &utmp[nutmp];
	while (--utp >= utmp) {
		if (!strncmp(utp->ut_name, name, sizeof(utmp[0].ut_name)))
			notify(utp, offset);
	}
}

/*
 * Check if a given tty name found in utmp is actually a legitimate tty.
 */
static int valid_tty(const char *line)
{
	const char *testline = line;

	/* 
	 * As of Linux 2.2 we can now sometimes have ttys in subdirs,
	 * but only /dev/pts. So if the name begins with pts/, skip that
	 * part. If there's a slash in anything else, complain loudly.
	 */
	if (!strncmp(testline, "pts/", 4)) testline += 4;

	if (strchr(testline, '/')) {
		/* A slash is an attempt to break security... */
		return 0;
	}
	return 1;
}

/*
 * This actually writes to the user's terminal.
 */
static void notify(struct utmp *utp, off_t offset)
{
	FILE *tp;                              /* file open on tty */
	struct stat stb;
	struct termios tbuf;
	char tty[sizeof(utp->ut_line)+sizeof(_PATH_DEV)+1]; /* full tty path */
	char name[sizeof(utp->ut_name) + 1];   /* user name */
	char line[sizeof(utp->ut_line) + 1];   /* tty name */
	const char *cr;                        /* line delimiter */
	sigset_t sigset;                       /* scratch signal mask */

#ifdef USER_PROCESS
	if (utp->ut_type != USER_PROCESS) {
		return;
	}
#endif

	/*
	 * Be careful in case we have a hostile utmp. (Remember sunos4 
	 * where it was mode 666?) Note that ut_line normally includes
	 * enough space for the null-terminator, but ut_name doesn't.
	 */
	strncpy(line, utp->ut_line, sizeof(utp->ut_line));
	line[sizeof(utp->ut_line)] = 0;

	strncpy(name, utp->ut_name, sizeof(utp->ut_name));
	name[sizeof(name) - 1] = '\0';

	/* Get the full path to the tty. */
	snprintf(tty, sizeof(tty), "%s%s", _PATH_DEV, line);

	/*
	 * Again, in case we have a hostile utmp, try to make sure we've
	 * got valid data and we're not being asked to write to, say,
	 * /etc/passwd.
	 */
	if (!valid_tty(line)) {
		syslog(LOG_AUTH | LOG_NOTICE, "invalid tty \"%s\" in utmp", 
		       tty);
		return;
	}

	/*
	 * If the user's tty isn't mode u+x, they don't want biffage;
	 * skip them.
	 */
	if (stat(tty, &stb) || !(stb.st_mode & S_IEXEC)) {
		dsyslog(LOG_DEBUG, "%s: wrong mode on %s", name, tty);
		return;
	}
	dsyslog(LOG_DEBUG, "notify %s on %s\n", name, tty);

	/*
	 * Fork a child process to do the tty I/O. There are, traditionally, 
	 * various ways for tty I/O to block indefinitely or otherwise hang.
	 * Also later down we're going to setuid to make sure we can't be
	 * used to read any file on the system.
	 *
	 * This of course makes a denial of service attack possible. So,
	 * in the parent process, wait a tenth of a second before continuing.
	 * This causes a primitive form of rate-limiting. It might be too 
	 * much delay for a busy system. But a busy system probably shouldn't
	 * be using comsat. If someone would like to make this smarter, feel
	 * free.
	 *
	 * Note: sleep with select() and NOT sleep() because on some systems
	 * at least sleep uses alarm(), which would interfere with our 
	 * reload/timeout logic.
	 */
	if (fork()) {
		/* parent process */
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		select(0, NULL, NULL, NULL, &tv);
		return;
	}
	/* child process */

	/*
	 * Turn off the alarm handler and instead exit in thirty seconds.
	 * Make sure SIGALRM isn't blocked.
	 */
	signal(SIGALRM, SIG_DFL);
	sigemptyset(&sigset);
	sigprocmask(SIG_SETMASK, &sigset, NULL);
	alarm(30);

	/*
	 * Open the tty.
	 */
	if ((tp = fopen(tty, "w")) == NULL) {
		dsyslog(LOG_ERR, "fopen of tty %s failed", tty);
		_exit(-1);
	}

	/* 
	 * Get the tty mode. Try to determine whether to use \n or \r\n 
	 * for linebreaks.
	 */
	tcgetattr(fileno(tp), &tbuf);
	if ((tbuf.c_oflag & OPOST) && (tbuf.c_oflag & ONLCR)) cr = "\n";
	else cr = "\r\n";

	/*
	 * Announce the mail.
	 */
	fprintf(tp, "%s\aNew mail for %s@%.*s\a has arrived:%s----%s",
		cr, name, (int)sizeof(hostname), hostname, cr, cr);

	/*
	 * Print the first few lines of the message.
	 */
	jkfprintf(tp, name, offset, cr);

	/*
	 * Close up and quit the child process.
	 */
	fclose(tp);
	_exit(0);
}

/*
 * This prints a few lines from the mailbox of the user "name" at offset
 * "offset", using "cr" as the line break string, to the file "tp".
 */
static void jkfprintf(FILE *tp, const char *name, off_t offset, const char *cr)
{
	char *cp, ch;
	FILE *fi;
	int linecnt, charcnt, inheader;
	struct passwd *p;
	char line[BUFSIZ];

	/* 
	 * Set uid to user in case mail drop is on nfs 
	 * ...and also to prevent cute symlink to /etc/shadow games 
	 */
	if ((p = getpwnam(name)) == NULL) {
		/*
		 * If user is not in passwd file, assume that it's
		 * an attempt to break security...
		 */
		syslog(LOG_AUTH | LOG_NOTICE, "%s not in passwd file", name);
		return;
	}

	/* 
	 * This sets both real and effective uids and clears the saved uid 
	 * too (see posix) so it's a one-way trip.
	 */
	setuid(p->pw_uid);

	/*
	 * Open the user's mailbox (recall we're already in the mail spool dir)
	 */
	fi = fopen(name, "r");
	if (fi == NULL)	return;

	/* Move to requested offset */
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
