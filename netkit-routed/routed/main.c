/*
 * Copyright (c) 1983, 1988, 1993
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
  "@(#) Copyright (c) 1983, 1988, 1993\n"
  "      The Regents of the University of California.  All rights reserved.\n";

/*
 * From: @(#)main.c	5.23 (Berkeley) 7/1/91
 * From: @(#)main.c	8.1 (Berkeley) 6/5/93
 */
char main_rcsid[] = 
  "$Id: main.c,v 1.7 1997/05/19 09:22:29 dholland Exp $";


/*
 * Routing Table Management Daemon
 */

#include "defs.h"
#include <sys/ioctl.h>
#include <sys/file.h>

#include <net/if.h>

#include <errno.h>
#include <signal.h>
#include <syslog.h>
#include "pathnames.h"
#include <sys/utsname.h>

struct sockaddr_in addr;	/* address of daemon's socket */
int s;				/* source and sink of all data */
char	packet[MAXPACKETSIZE+1];
struct	servent *sp;

int	supplier = -1;		/* process should supply updates */
int	gateway = 0;		/* 1 if we are a gateway to parts beyond */
int	debug = 0;
int	bufspace = 127*1024;	/* max. input buffer size to request */
struct	rip *msg = (struct rip *)packet;
int	kernel_version;
void	getkversion(void);

int getsocket(int, int, struct sockaddr *);
void process(int);

int main(int argc, char *argv[])
{
	int n, nfd, tflags = 0;
	struct timeval *tvp, waittime;
	struct itimerval itval;
	struct rip *query = msg;
	fd_set ibits;
	sigset_t sigset, osigset;
	
	openlog("routed", LOG_PID | LOG_ODELAY, LOG_DAEMON);
	setlogmask(LOG_UPTO(LOG_WARNING));
	getkversion();

	sp = getservbyname("router", "udp");
	if (sp == NULL) {
		fprintf(stderr, "routed: router/udp: unknown service\n");
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = sp->s_port;

	s = getsocket(AF_INET, SOCK_DGRAM, (struct sockaddr *)&addr);
	if (s < 0)
		exit(1);
	argv++, argc--;
	while (argc > 0 && **argv == '-') {
		if (strcmp(*argv, "-s") == 0) {
			supplier = 1;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-q") == 0) {
			supplier = 0;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-t") == 0) {
			tflags++;
			setlogmask(LOG_UPTO(LOG_DEBUG));
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-d") == 0) {
			debug++;
			setlogmask(LOG_UPTO(LOG_DEBUG));
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-g") == 0) {
			gateway = 1;
			argv++, argc--;
			continue;
		}
		fprintf(stderr,
			"usage: routed [ -s ] [ -q ] [ -t ] [ -g ]\n");
		exit(1);
	}

	if (debug == 0 && tflags == 0)
	{
		if (fork() != 0) exit(0);
		close(0);
		close(1);
		close(2);
		setsid();
	}

	/*
	 * Any extra argument is considered
	 * a tracing log file.
	 */

	if (argc > 0)
		traceon(*argv);
	while (tflags-- > 0)
		bumploglevel();

	(void) gettimeofday(&now, (struct timezone *)NULL);

	/*
	 * Collect an initial view of the world by
	 * checking the interface configuration and the gateway kludge
	 * file.  Then, send a request packet on all
	 * directly connected networks to find out what
	 * everyone else thinks.
	 */

	rtinit();
	ifinit();
	gwkludge();
	if (gateway > 0)
		rtdefault();
	if (supplier < 0)
		supplier = 0;
	query->rip_cmd = RIPCMD_REQUEST;
	query->rip_vers = RIPVERSION;
	if (sizeof(query->rip_nets[0].rip_dst.sa_family) > 1)	/* XXX */
		query->rip_nets[0].rip_dst.sa_family = htons((u_short)AF_UNSPEC);
	else
		query->rip_nets[0].rip_dst.sa_family = AF_UNSPEC;
	query->rip_nets[0].rip_metric = htonl((u_long)HOPCNT_INFINITY);
	toall(sndmsg, 0, NULL);
	signal(SIGALRM, timer);
	signal(SIGHUP, hup);
	signal(SIGTERM, hup);
	signal(SIGINT, rtdeleteall);
	signal(SIGUSR1, sigtrace);
	signal(SIGUSR2, sigtrace);
	itval.it_interval.tv_sec = TIMER_RATE;
	itval.it_value.tv_sec = TIMER_RATE;
	itval.it_interval.tv_usec = 0;
	itval.it_value.tv_usec = 0;
	srandom(getpid());
	if (setitimer(ITIMER_REAL, &itval, (struct itimerval *)NULL) < 0)
		syslog(LOG_ERR, "setitimer: %m\n");

	FD_ZERO(&ibits);
	nfd = s + 1;			/* 1 + max(fd's) */
	for (;;) {
		FD_SET(s, &ibits);
		/*
		 * If we need a dynamic update that was held off,
		 * needupdate will be set, and nextbcast is the time
		 * by which we want select to return.  Compute time
		 * until dynamic update should be sent, and select only
		 * until then.  If we have already passed nextbcast,
		 * just poll.
		 */
		if (needupdate) {
			waittime = nextbcast;
			timevalsub(&waittime, &now);
			if (waittime.tv_sec < 0) {
				waittime.tv_sec = 0;
				waittime.tv_usec = 0;
			}
			if (traceactions)
				fprintf(ftrace,
				 "select until dynamic update %d/%d sec/usec\n",
				    waittime.tv_sec, waittime.tv_usec);
			tvp = &waittime;
		} else
			tvp = (struct timeval *)NULL;
		n = select(nfd, &ibits, 0, 0, tvp);
		if (n <= 0) {
			/*
			 * Need delayed dynamic update if select returned
			 * nothing and we timed out.  Otherwise, ignore
			 * errors (e.g. EINTR).
			 */
			if (n < 0) {
				if (errno == EINTR)
					continue;
				syslog(LOG_ERR, "select: %m");
			}
			sigemptyset(&sigset);
			sigaddset(&sigset, SIGALRM);
			sigprocmask(SIG_BLOCK, &sigset, &osigset);
			if (n == 0 && needupdate) {
				if (traceactions)
					fprintf(ftrace,
					    "send delayed dynamic update\n");
				(void) gettimeofday(&now,
					    (struct timezone *)NULL);
				toall(supply, RTS_CHANGED,
				    (struct interface *)NULL);
				lastbcast = now;
				needupdate = 0;
				nextbcast.tv_sec = 0;
			}
			sigprocmask(SIG_SETMASK, &osigset, NULL);
			continue;
		}
		(void) gettimeofday(&now, (struct timezone *)NULL);
		sigemptyset(&sigset);
		sigaddset(&sigset, SIGALRM);
		sigprocmask(SIG_BLOCK, &sigset, &osigset);
		if (ibits.fds_bits[s/32] & (1 << s))
			process(s);
		/* handle ICMP redirects */
		sigprocmask(SIG_SETMASK, &osigset, NULL);
	}
}

/*
 * Put Linux kernel version into
 * the global variable kernel_version.
 * Example: 1.2.8 = 0x010208
 */

void getkversion(void)
{
  struct utsname uts;
  int maj, min, pl;

  maj = min = pl = 0;
  uname(&uts);
  sscanf(uts.release, "%d.%d.%d", &maj, &min, &pl);
  kernel_version = (maj << 16) | (min << 8) | pl;
}

void timevaladd(struct timeval *t1, struct timeval *t2)
{

	t1->tv_sec += t2->tv_sec;
	if ((t1->tv_usec += t2->tv_usec) > 1000000) {
		t1->tv_sec++;
		t1->tv_usec -= 1000000;
	}
}

void timevalsub(struct timeval *t1, struct timeval *t2)
{

	t1->tv_sec -= t2->tv_sec;
	if ((t1->tv_usec -= t2->tv_usec) < 0) {
		t1->tv_sec--;
		t1->tv_usec += 1000000;
	}
}

void process(int fd)
{
	struct sockaddr from;
	size_t fromlen;
	int cc;
	union {
		char	buf[MAXPACKETSIZE+1];
		struct	rip rip;
	} inbuf;

	for (;;) {
		fromlen = sizeof (from);
		cc = recvfrom(fd, &inbuf, sizeof (inbuf), 0, &from, &fromlen);
		if (cc <= 0) {
			if (cc < 0 && errno != EWOULDBLOCK)
				perror("recvfrom");
			break;
		}
		if (fromlen != sizeof (struct sockaddr_in))
			break;
		rip_input(&from, &inbuf.rip, cc);
	}
}

int getsocket(int domain, int type, struct sockaddr *sa)
{
	struct sockaddr_in *sin=(struct sockaddr_in *)sa;
	int sock, on = 1;

	if ((sock = socket(domain, type, 0)) < 0) {
		perror("socket");
		syslog(LOG_ERR, "socket: %m");
		return (-1);
	}
#ifdef SO_BROADCAST
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0) {
		syslog(LOG_ERR, "setsockopt SO_BROADCAST: %m");
		close(sock);
		return (-1);
	}
#endif
#ifdef SO_RCVBUF
	for (on = bufspace; ; on -= 1024) {
		if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
		    &on, sizeof (on)) == 0)
			break;
		if (on <= 8*1024) {
			syslog(LOG_ERR, "setsockopt SO_RCVBUF: %m");
			break;
		}
	}
	if (traceactions)
		fprintf(ftrace, "recv buf %d\n", on);
#endif
	if (bind(sock, (struct sockaddr *)sin, sizeof (*sin)) < 0) {
		perror("bind");
		syslog(LOG_ERR, "bind: %m");
		close(sock);
		return (-1);
	}
	if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1)
		syslog(LOG_ERR, "fcntl O_NONBLOCK: %m\n");
	return (sock);
}