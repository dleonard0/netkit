/*
 * Copyright (c) 1983 The Regents of the University of California.
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
  "@(#) Copyright (c) 1983 The Regents of the University of California.\n"
  "All rights reserved.\n";

/* 
 * from: @(#)fingerd.c	5.6 (Berkeley) 6/1/90"
 */
char rcsid[] = 
  "$Id: fingerd.c,v 1.12 1996/12/30 07:58:07 dholland Exp $";


#include <pwd.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <getopt.h>
#include <netinet/in.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include "pathnames.h"

#define	ENTRIES	50
#define WS " \t\r\n"

/* These are used in this order if the finger path compiled in doesn't work. */
#define _ALT_PATH_FINGER_1 "/usr/local/bin/finger"
#define _ALT_PATH_FINGER_2 "/usr/ucb/finger"
#define _ALT_PATH_FINGER_3 "/usr/bin/finger"

static
void
fatal(const char *msg, int logit)
{
	const char *err = strerror(errno);
	if (logit) syslog(LOG_ERR, "%s: %s:\n", msg, err);
	fprintf(stderr, "fingerd: %s: %s\r\n", msg, err);
	exit(1);
}


int
main(int argc, char *argv[])
{
	struct passwd *pw;
	FILE *fp;
	int ch, ca;
	int p[2];
	const char *av[ENTRIES + 1];
	char line[1024];
	int welcome = 0, heavylogging = 0, nouserlist = 0;
	int forwarding = 0;
	int k, pid, nusers;
	char *s, *t;
	const char *fingerpath = NULL;


	struct sockaddr_in sn;
	size_t sval = sizeof(sn);
	if (getpeername(0, (struct sockaddr *) &sn, &sval) < 0) {
		fatal("getpeername", 0);
	}

	if ((pw = getpwnam("nobody")) != NULL) {
	        setgid(pw->pw_gid);
		setuid(pw->pw_uid);
	}

	opterr = 0;
	while ((ca = getopt(argc, argv, "wlLpufh?")) != EOF) {
		switch(ca) {
		  case 'w':
			welcome = 1;
			break;
		  case 'l':
		        heavylogging = 1;
			break;
		  case 'L': 
		  case 'p':
		        fingerpath = optarg;
			break;
		  case 'u':
		        nouserlist = 1;
			break;
		  case 'f':
			forwarding = 1;
			break;
		  case '?':
		  case 'h':
		  default:
			syslog(LOG_ERR, "usage: fingerd [-lLwuf]");
			exit(1);
		}
	}
	argc -= optind;
	argv += optind;

	openlog("fingerd", LOG_PID, LOG_DAEMON);

	if (!fgets(line, sizeof(line), stdin))
		exit(1);

	if (welcome) {
		char buf[256];
		struct hostent *hp;
		struct utsname utsname;

		uname(&utsname);
		gethostname(buf, sizeof(buf));
		if ((hp = gethostbyname(buf))) {
			/* paranoia: dns spoofing? */
			strncpy(buf, hp->h_name, sizeof(buf));
			buf[sizeof(buf)-1] = 0;
		}
		printf("\r\nWelcome to %s version %s at %s !\r\n\n",
				utsname.sysname, utsname.release, buf);
		fflush(stdout);
		switch (fork()) {
		 case -1: /* fork failed, oh well */
		     break;
		 case 0: /* child */
		     execl(_PATH_UPTIME, _PATH_UPTIME, NULL);
		     _exit(1);
		 default: /* parent */
		     wait(NULL);
		     break;
		}
		fflush(stdout);
		printf("\r\n");
		fflush(stdout);
	}

	k = nusers = 0;
	av[k++] = "finger";
	for (s = strtok(line, WS); s && k<ENTRIES; s = strtok(NULL, WS)) {
		/* RFC742: "/[Ww]" == "-l" */
		if (!strncasecmp(s, "/w", 2)) memcpy(s, "-l", 2);
		t = strchr(s, '@');
		if (t && !forwarding) {
			fprintf(stderr, "fingerd: forwarding not allowed\r\n");
			syslog(LOG_WARNING, "rejected %s\n", s);
			exit(1);
		}
		if (heavylogging) {
		    if (*s=='-') syslog(LOG_INFO, "option %s\n", s);
		    else syslog(LOG_INFO, "fingered %s\n", s);
		}
		av[k++] = s;
		if (*s!='-') nusers++;
	}
	av[k] = NULL;
	if (nusers==0) {
		/* finger @host */
		if (nouserlist) {
			syslog(LOG_WARNING, "rejected finger @host\n");
			printf("Please supply a username\r\n");
			return 0;
		}
		if (heavylogging) syslog(LOG_INFO, "fingered @host\n");
	}

	if (pipe(p) < 0) fatal("pipe", 1);

	pid = fork();
	if (pid<0) fatal("fork", 1);
	if (pid==0) {
		/* child */
		close(p[0]);
		dup2(p[1], 1);
		if (p[1]!=1) close(p[1]);

		if (fingerpath) execv(fingerpath, (char **)av);
		execv(_PATH_FINGER, (char **)av);
		execv(_ALT_PATH_FINGER_1, (char **)av);
		execv(_ALT_PATH_FINGER_2, (char **)av);
		execv(_ALT_PATH_FINGER_3, (char **)av);
		_exit(1);
	}
	/* parent */
	close(p[1]);

	/* convert \n to \r\n. This should be an option to finger... */
	fp = fdopen(p[0], "r");
	if (!fp) fatal("fdopen", 1);

	while ((ch = getc(fp)) != EOF) {
		if (ch == '\n')	putchar('\r');
		putchar(ch);
	}
	return 0;
}

