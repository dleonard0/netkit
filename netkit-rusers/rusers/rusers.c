/*-
 *  Copyright (c) 1993 John Brezak
 *  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef lint
char rusers_rcsid[] = 
  "$Id: rusers.c,v 1.17 1999/12/12 19:32:05 dholland Exp $";
#endif /* not lint */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <arpa/inet.h>
#include <utmp.h>
#include <stdlib.h>
#include <unistd.h>

#include "../version.h"

/*
 * For now we only try version 2 of the protocol. The current
 * version is 3 (rusers.h), but only Solaris and NetBSD seem
 * to support it currently.
 */
/*#include <rpcsvc/rnusers.h>*/	/* Old version */

#include "rusers.h"  /* get the one we just built with rpcgen */

/*
 * Sigh.
 */
#ifdef GNU_LIBC
	#define RUT_TIME ut_time
#else
	#define RUT_TIME ut_time
#endif

#define MAX_INT 0x7fffffff
#define HOST_WIDTH 20
#define LINE_WIDTH 8
char *argv0;

struct timeval timeout = { 25, 0 };
int longopt;
int allopt;

struct host_list {
	struct host_list *next;
	struct in_addr addr;
} *hosts;

static
int
search_host(struct in_addr addr)
{
	struct host_list *hp;
	
	if (!hosts)
		return(0);

	for (hp = hosts; hp != NULL; hp = hp->next) {
		if (hp->addr.s_addr == addr.s_addr)
			return(1);
	}
	return(0);
}

static
void
remember_host(struct in_addr addr)
{
	struct host_list *hp;

	if (!(hp = (struct host_list *)malloc(sizeof(struct host_list)))) {
		fprintf(stderr, "%s: no memory.\n", argv0);
		exit(1);
	}
	hp->addr.s_addr = addr.s_addr;
	hp->next = hosts;
	hosts = hp;
}

static
int
rusers_reply(char *replyp, struct sockaddr_in *raddrp)
{
	int x, idlee;
	char date[32], idle_time[64], remote[64], local[64];
	struct hostent *hp;
	struct utmpidlearr *up = (struct utmpidlearr *)replyp;
	const char *host;
	int days, hours, minutes, seconds;
	
	if (search_host(raddrp->sin_addr))
		return(0);

	if (!allopt && !up->uia_cnt)
		return(0);
	
	hp = gethostbyaddr((char *)&raddrp->sin_addr.s_addr,
			   sizeof(struct in_addr), AF_INET);
	if (hp)
		host = hp->h_name;
	else
		host = inet_ntoa(raddrp->sin_addr);
	
	if (!longopt)
		printf("%-*.*s ", HOST_WIDTH, HOST_WIDTH, host);
	
	for (x = 0; x < up->uia_cnt; x++) {
		time_t tmptime = up->uia_arr[x]->ui_utmp.RUT_TIME;
		strncpy(date, ctime(&tmptime) + 4, sizeof(date)-1);
		date[sizeof(date)-1] = 0;

		idlee = up->uia_arr[x]->ui_idle;
		snprintf(idle_time, sizeof(idle_time), "   :%02d", idlee);
		if (idlee == MAX_INT)
			strcpy(idle_time, "??");
		else if (idlee == 0)
			strcpy(idle_time, "");
		else {
			seconds = idlee;
			days = seconds/(60*60*24);
			seconds %= (60*60*24);
			hours = seconds/(60*60);
			seconds %= (60*60);
			minutes = seconds/60;
			seconds %= 60;
			if (idlee > 60)
				snprintf(idle_time, sizeof(idle_time),
					 "%2d:%02d", minutes, seconds);
			if (idlee >= (60*60))
				snprintf(idle_time, sizeof(idle_time),
					 "%2d:%02d:%02d",
					 hours, minutes, seconds);
			if (idlee >= (24*60*60))
				snprintf(idle_time, sizeof(idle_time),
					 "%d days, %d:%02d:%02d",
					 days, hours, minutes, seconds);
		}

		strncpy(remote, up->uia_arr[x]->ui_utmp.ut_host,
			sizeof(remote)-1);
		remote[sizeof(remote)-1] = 0;

		if (strlen(remote) != 0)
			snprintf(remote, sizeof(remote), "(%.16s)",
			    up->uia_arr[x]->ui_utmp.ut_host);

		if (longopt) {
			/* Fit into HOST_WIDTH+LINE_WIDTH+1 chars */
			int len1 = strlen(host);
			int len2 = strlen(up->uia_arr[x]->ui_utmp.ut_line);
			if (len1 + len2 > HOST_WIDTH+LINE_WIDTH+1) {
			    int excess = len1 + len2 - HOST_WIDTH-LINE_WIDTH-1;
			    if (excess < len1) len1 -= excess;
			    else if (excess < len2) len2 -= excess;
			    else {
				/* Hmm. Probably an attack... */
				len1 = HOST_WIDTH;
				len2 = LINE_WIDTH;
			    }
			}
			snprintf(local, sizeof(local),
				 "%-.*s:%-.*s", len1, host, len2,
				 up->uia_arr[x]->ui_utmp.ut_line);

			printf("%-8.8s %-*.*s %-12.12s %8s %.18s\n",
			    up->uia_arr[x]->ui_utmp.ut_name,
			    HOST_WIDTH+LINE_WIDTH+1, HOST_WIDTH+LINE_WIDTH+1, local,
			    date,
			    idle_time,
			    remote);
		} else
			printf("%.8s ",
			    up->uia_arr[x]->ui_utmp.ut_name);
	}
	if (!longopt)
		putchar('\n');
	
	remember_host(raddrp->sin_addr);
	return(0);
}

static
void
onehost(char *host)
{
	struct utmpidlearr up;
	CLIENT *rusers_clnt;
	struct sockaddr_in addr;
	struct hostent *hp;
	
	hp = gethostbyname(host);
	if (hp == NULL) {
		fprintf(stderr, "%s: unknown host \"%s\"\n",
			argv0, host);
		exit(1);
	}

	rusers_clnt = clnt_create(host, RUSERSPROG, RUSERSVERS_IDLE, "udp");
	if (rusers_clnt == NULL) {
		clnt_pcreateerror(argv0);
		exit(1);
	}

	memset(&up, 0, sizeof(up));
	memset(&addr, 0, sizeof(addr));
	if (clnt_call(rusers_clnt, RUSERSPROC_NAMES, (xdrproc_t)xdr_void, NULL,
	    (xdrproc_t) xdr_utmpidlearr, &up, timeout) != RPC_SUCCESS) {
		clnt_perror(rusers_clnt, argv0);
		exit(1);
	}
	if (hp->h_length > (int)sizeof(addr.sin_addr)) {
		hp->h_length = sizeof(addr.sin_addr);
	}
	memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
	rusers_reply((char *)&up, &addr);
}

static
void
allhosts(void)
{
	struct utmpidlearr up;
	enum clnt_stat clnt_stat;

	bzero((char *)&up, sizeof(up));
	clnt_stat = clnt_broadcast(RUSERSPROG, RUSERSVERS_IDLE,
				   RUSERSPROC_NAMES, 
				   (xdrproc_t) xdr_void, NULL, 
				   (xdrproc_t) xdr_utmpidlearr,
				   (char *) &up, 
				   (resultproc_t) rusers_reply);
	if (clnt_stat != RPC_SUCCESS && clnt_stat != RPC_TIMEDOUT) {
		fprintf(stderr, "%s: %s\n", argv0, clnt_sperrno(clnt_stat));
		exit(1);
	}
}

static
void usage(void)
{
	fprintf(stderr, "Usage: %s [-la] [hosts ...]\n", argv0);
	exit(1);
}

int main(int argc, char *argv[])
{
	int ch;
	
	if (!(argv0 = rindex(argv[0], '/')))
		argv0 = argv[0];
	else
		argv0++;


	while ((ch = getopt(argc, argv, "al")) != -1)
		switch (ch) {
		case 'a':
			allopt++;
			break;
		case 'l':
			longopt++;
			break;
		default:
			usage();
			/*NOTREACHED*/
		}

	setlinebuf(stdout);
	if (argc == optind)
		allhosts();
	else {
		for (; optind < argc; optind++)
			(void) onehost(argv[optind]);
	}
	exit(0);
}
