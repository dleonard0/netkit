/*-
 * Copyright (c) 1993, John Brezak
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

char rcsid[] = "$Id: rup.c,v 1.2 1996/07/15 21:28:13 dholland Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netdb.h>
#include <rpc/rpc.h>
#include <arpa/inet.h>
#include <rpc/pmap_clnt.h>
#include <err.h>

#undef FSHIFT			/* Use protocol's shift and scale values */
#undef FSCALE
#include <rpcsvc/rstat.h>

#define HOST_WIDTH 24

static int print_rup_data(const char *host, statstime *host_stat);

int printtime;			/* print the remote host(s)'s time */

struct host_list {
	struct host_list *next;
	struct in_addr addr;
} *hosts;

int
search_host(addr)
	struct in_addr addr;
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

void
remember_host(addr)
	struct in_addr addr;
{
	struct host_list *hp;

	if (!(hp = (struct host_list *)malloc(sizeof(struct host_list)))) {
		err(1, NULL);
		/* NOTREACHED */
	}
	hp->addr.s_addr = addr.s_addr;
	hp->next = hosts;
	hosts = hp;
}


struct rup_data {
	char *host;
	struct statstime statstime;
};
struct rup_data *rup_data;
int rup_data_idx = 0;
int rup_data_max = 0;

enum sort_type { 
	SORT_NONE,
	SORT_HOST,
	SORT_LDAV,
	SORT_UPTIME
};
enum sort_type sort_type;

static int
compare(const void *a1, const void *a2)
{
	const struct rup_data *d1 = (struct rup_data *) a1;
	const struct rup_data *d2 = (struct rup_data *) a2;

	switch(sort_type) {
	case SORT_HOST:
		return strcmp(d1->host, d2->host);
	case SORT_LDAV:
		return d1->statstime.avenrun[0] 
			- d2->statstime.avenrun[0];
	case SORT_UPTIME:
		return d1->statstime.boottime.tv_sec 
			- d2->statstime.boottime.tv_sec;
	default:
		/* something's really wrong here */
		abort();
	}
}

void
remember_rup_data(host, st)
	char *host;
	struct statstime *st;
{
        if (rup_data_idx >= rup_data_max) {
                rup_data_max += 16;
                rup_data = realloc (rup_data, 
				rup_data_max * sizeof(struct rup_data));
                if (rup_data == NULL) {
                        err (1, NULL);
			/* NOTREACHED */
                }
        }
	
	rup_data[rup_data_idx].host = strdup(host);
	rup_data[rup_data_idx].statstime = *st;
	rup_data_idx++;
}


int
rstat_reply(replyp, raddrp)
	char *replyp;
	struct sockaddr_in *raddrp;
{
	struct hostent *hp;
	const char *host;
	statstime *host_stat = (statstime *)replyp;

	if (!search_host(raddrp->sin_addr)) {
		hp = gethostbyaddr((char *)&raddrp->sin_addr.s_addr,
			sizeof(struct in_addr), AF_INET);

		if (hp)
			host = hp->h_name;
		else
			host = inet_ntoa(raddrp->sin_addr);

		remember_host(raddrp->sin_addr);

		if (sort_type != SORT_NONE) {
			remember_rup_data(host, host_stat);
		} else {
			print_rup_data(host, host_stat);
		}
	}

	return 0;
}

static int
print_rup_data(const char *host, statstime *host_stat)
{
	struct tm *tmp_time;
	struct tm host_time;
	struct tm host_uptime;
	char days_buf[16];
	char hours_buf[16];

	printf("%-*.*s", HOST_WIDTH, HOST_WIDTH, host);

	tmp_time = localtime((time_t *)&host_stat->curtime.tv_sec);
	host_time = *tmp_time;

	host_stat->curtime.tv_sec -= host_stat->boottime.tv_sec;

	tmp_time = gmtime((time_t *)&host_stat->curtime.tv_sec);
	host_uptime = *tmp_time;

	if (host_uptime.tm_yday != 0)
		sprintf(days_buf, "%3d day%s, ", host_uptime.tm_yday,
			(host_uptime.tm_yday > 1) ? "s" : "");
	else
		days_buf[0] = '\0';

	if (host_uptime.tm_hour != 0)
		sprintf(hours_buf, "%2d:%02d, ",
			host_uptime.tm_hour, host_uptime.tm_min);
	else
		if (host_uptime.tm_min != 0)
			sprintf(hours_buf, "%2d mins, ", host_uptime.tm_min);
		else
			hours_buf[0] = '\0';

	if (printtime)
		printf(" %2d:%02d%cm", host_time.tm_hour % 12,
			host_time.tm_min,
			(host_time.tm_hour >= 12) ? 'p' : 'a');

	printf(" up %9.9s%9.9s load average: %.2f %.2f %.2f\n",
		days_buf, hours_buf,
		(double)host_stat->avenrun[0]/FSCALE,
		(double)host_stat->avenrun[1]/FSCALE,
		(double)host_stat->avenrun[2]/FSCALE);

	return(0);
}


void
onehost(host)
	char *host;
{
	CLIENT *rstat_clnt;
	statstime host_stat;
	struct timeval foo;
	
	rstat_clnt = clnt_create(host, RSTATPROG, RSTATVERS_TIME, "udp");
	if (rstat_clnt == NULL) {
		warnx("%s", clnt_spcreateerror(host));
		return;
	}

	memset(&host_stat, 0, sizeof(host_stat));
	if (clnt_call(rstat_clnt, RSTATPROC_STATS, 
		      (xdrproc_t) xdr_void, NULL, 
		      (xdrproc_t) xdr_statstime, &host_stat, 
		      foo) != RPC_SUCCESS) 
	{
		warnx("%s",  clnt_sperror(rstat_clnt, host));
		return;
	}

	print_rup_data(host, &host_stat);
	clnt_destroy(rstat_clnt);
}

void
allhosts()
{
	statstime host_stat;
	enum clnt_stat clnt_stat;
	size_t i;

	if (sort_type != SORT_NONE) {
		printf("collecting responses...");
		fflush(stdout);
	}

	clnt_stat = clnt_broadcast(RSTATPROG, RSTATVERS_TIME, RSTATPROC_STATS,
				   (xdrproc_t) xdr_void, NULL,
				   (xdrproc_t) xdr_statstime, &host_stat, 
				   (resultproc_t) rstat_reply);
	if (clnt_stat != RPC_SUCCESS && clnt_stat != RPC_TIMEDOUT) {
		warnx("%s", clnt_sperrno(clnt_stat));
		exit(1);
	}

	if (sort_type != SORT_NONE) {
		putchar('\n');
		qsort(rup_data, rup_data_idx, sizeof(struct rup_data), compare);

		for (i = 0; i < rup_data_idx; i++) {
			print_rup_data(rup_data[i].host, &rup_data[i].statstime);
		}
	}
}

static void usage(void)
{
	fprintf(stderr, "Usage: rup [-dhlt] [hosts ...]\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int ch;
	extern int optind;

	sort_type = SORT_NONE;
	while ((ch = getopt(argc, argv, "dhlt")) != -1)
		switch (ch) {
		case 'd':
			printtime = 1;
			break;
		case 'h':
			sort_type = SORT_HOST;
			break;
		case 'l':
			sort_type = SORT_LDAV;
			break;
		case 't':
			sort_type = SORT_UPTIME;
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
			onehost(argv[optind]);
	}

	exit(0);
}
