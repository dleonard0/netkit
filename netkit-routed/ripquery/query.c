/*-
 * Copyright (c) 1982, 1986, 1993
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
  "@(#) Copyright (c) 1982, 1986, 1993\n"
  "    The Regents of the University of California.  All rights reserved.\n";

/*
 * From: @(#)query.c	5.13 (Berkeley) 4/16/91
 * From: query.c,v 1.4 1993/08/01 18:24:51 mycroft Exp 
 * From: @(#)query.c	8.1 (Berkeley) 6/5/93
 */
char rcsid[] = 
  "$Id: query.c,v 1.8 1999/09/29 01:47:35 netbug Exp $";

#include "../version.h"

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
#include <netinet/in.h>
#include <protocols/routed.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define	WTIME	5		/* Time to wait for all responses */
#define	STIME	500000		/* usec to wait for another response */

int	s;
volatile int timedout;
void	timeout(int);
void 	query(char *host);
void	rip_input(struct sockaddr *from, int size);
int 	inet_subnetof(struct in_addr in);
char	packet[MAXPACKETSIZE];
int	nflag;

int main(int argc, char *argv[])
{
	int ch, cc, count=0, bits;
	struct sockaddr from;
	struct sigaction sigact;
	socklen_t fromlen = sizeof(from);
	int size = 32*1024;
	struct timeval shorttime;

	while ((ch = getopt(argc, argv, "n")) != EOF)
		switch (ch) {
		case 'n':
			nflag++;
			break;
		case '?':
		default:
			goto usage;
		}
	argv += optind;

	if (!*argv) {
usage:		printf("usage: ripquery [-n] hosts...\n");
		exit(1);
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		exit(2);
	}
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) < 0)
		perror("setsockopt SO_RCVBUF");

	while (*argv) {
		query(*argv++);
		count++;
	}

	/*
	 * Listen for returning packets;
	 * may be more than one packet per host.
	 */
	bits = 1 << s;
	memset(&shorttime, 0, sizeof(shorttime));
	shorttime.tv_usec = STIME;
	memset(&sigact, 0, sizeof(sigact));
	sigact.sa_handler = timeout;
	/*sigact.sa_flags = 0;*/		/* no restart */
	if (sigaction(SIGALRM, &sigact, (struct sigaction *)NULL) == -1)
		perror("sigaction");
	alarm(WTIME);
	while ((count > 0 && !timedout) ||
	    select(20, (fd_set *)&bits, NULL, NULL, &shorttime) > 0) {
		cc = recvfrom(s, packet, sizeof (packet), 0,
		  &from, &fromlen);
		if (cc <= 0) {
			if (cc < 0) {
				if (errno == EINTR)
					continue;
				perror("recvfrom");
				(void) close(s);
				exit(1);
			}
			continue;
		}
		rip_input(&from, cc);
		count--;
	}
	exit (count > 0 ? count : 0);
}

void query(char *host)
{
	struct sockaddr_in router;
	register struct rip *msg = (struct rip *)packet;
	struct hostent *hp;
	struct servent *sp;

	memset(&router, 0, sizeof (router));
	router.sin_family = AF_INET;
	if (inet_aton(host, &router.sin_addr) == 0) {
		hp = gethostbyname(host);
		if (hp == NULL) {
			fprintf(stderr, "ripquery: %s: ", host);
			herror((char *)NULL);
			exit(1);
		}
		memcpy(&router.sin_addr, hp->h_addr, sizeof(router.sin_addr));
	}
	sp = getservbyname("router", "udp");
	if (sp == 0) {
		printf("udp/router: service unknown\n");
		exit(1);
	}
	router.sin_port = sp->s_port;
	msg->rip_cmd = RIPCMD_REQUEST;
	msg->rip_vers = RIPVERSION;
	msg->rip_nets[0].rip_dst.sa_family = htons(AF_UNSPEC);
	msg->rip_nets[0].rip_metric = htonl(HOPCNT_INFINITY);
	if (sendto(s, packet, sizeof (struct rip), 0,
	  (struct sockaddr *)&router, sizeof(router)) < 0)
		perror(host);
}

/*
 * Handle an incoming routing packet.
 */
 
void rip_input(struct sockaddr *sa, int size)
{
	struct sockaddr_in *from = (struct sockaddr_in *)sa;
	register struct rip *msg = (struct rip *)packet;
	register struct netinfo *n;
	const char *name;
	int lna, net, subnet;
	struct hostent *hp;
	struct netent *np;

	if (msg->rip_cmd != RIPCMD_RESPONSE)
		return;
	printf("%d bytes from ", size);
	if (nflag)
		printf("%s:\n", inet_ntoa(from->sin_addr));
	else {
		hp = gethostbyaddr((char *)&from->sin_addr,
		    sizeof (struct in_addr), AF_INET);
		name = hp == 0 ? "???" : hp->h_name;
		printf("%s(%s):\n", name, inet_ntoa(from->sin_addr));
	}
	size -= sizeof (int);
	n = msg->rip_nets;
	while (size > 0) {
	    if (size < (int)sizeof (struct netinfo))
		    break;
	    if (msg->rip_vers > 0) {
		    n->rip_dst.sa_family =
			    ntohs(n->rip_dst.sa_family);
		    n->rip_metric = ntohl(n->rip_metric);
	    }
	    switch (n->rip_dst.sa_family) {

	    case AF_INET:
		{ register struct sockaddr_in *sin;

		sin = (struct sockaddr_in *)&n->rip_dst;
		net = inet_netof(sin->sin_addr);
		subnet = inet_subnetof(sin->sin_addr);
		lna = inet_lnaof(sin->sin_addr);
		name = "???";
		if (!nflag) {
			if (sin->sin_addr.s_addr == 0)
				name = "default";
			else if (lna == INADDR_ANY) {
				np = getnetbyaddr(net, AF_INET);
				if (np)
					name = np->n_name;
				else if (net == 0)
					name = "default";
			} else if ((lna & 0xff) == 0 &&
			    (np = getnetbyaddr(subnet, AF_INET))) {
				struct in_addr subnaddr;

				subnaddr = inet_makeaddr(subnet, INADDR_ANY);
				if (memcmp(&sin->sin_addr, &subnaddr,
				    sizeof(subnaddr)) == 0)
					name = np->n_name;
				else
					goto host;
			} else {
	host:
				hp = gethostbyaddr((char *)&sin->sin_addr,
				    sizeof (struct in_addr), AF_INET);
				if (hp)
					name = hp->h_name;
			}
			printf("\t%-17s metric %2d name %s\n",
				inet_ntoa(sin->sin_addr), n->rip_metric, name);
		} else
			printf("\t%-17s metric %2d\n",
				inet_ntoa(sin->sin_addr), n->rip_metric);
		break;
		}

	    default:
		{ u_short *p = (u_short *)n->rip_dst.sa_data;

		printf("\t(af %d) %x %x %x %x %x %x %x, metric %d\n",
		    p[0], p[1], p[2], p[3], p[4], p[5], p[6],
		    n->rip_dst.sa_family,
		    n->rip_metric);
		break;
		}
			
	    }
	    size -= sizeof (struct netinfo), n++;
	}
}

void timeout(int signum)
{
	(void)signum;

	timedout = 1;
}

/*
 * Return the possible subnetwork number from an internet address.
 * SHOULD FIND OUT WHETHER THIS IS A LOCAL NETWORK BEFORE LOOKING
 * INSIDE OF THE HOST PART.  We can only believe this if we have other
 * information (e.g., we can find a name for this number).
 */

int inet_subnetof(struct in_addr in)
{
	register u_long i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return ((i & IN_CLASSB_NET) >> IN_CLASSB_NSHIFT);
	else if (IN_CLASSB(i))
		return ((i & IN_CLASSC_NET) >> IN_CLASSC_NSHIFT);
	else
		return ((i & 0xffffffc0) >> 28);
}
