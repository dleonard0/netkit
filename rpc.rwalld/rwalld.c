/*
 * Copyright (c) 1993 Christopher G. Demetriou
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
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

char rcsid[] = 
  "$Id: rwalld.c,v 1.7 1996/08/15 07:02:31 dholland Exp $";

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>

/* work around bogus warning from linux includes */
#define __wait __wait_foo
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#undef __wait

#include "rwall.h"

/* from libbsd.a */
int daemon(int, int);

/*
 * Note: check to see if your wall accepts -n or not (check the source,
 * it's not documented) and modify as necessary. The -n flag is supposed
 * to suppress the banner.
 */
#ifdef OSF
#define WALL_CMD "/usr/sbin/wall"
#else
#define WALL_CMD "/usr/bin/wall -n"
#endif

void wallprog_1(struct svc_req *rqstp, SVCXPRT *transp);
void possess(void);
void killkids(int);

int nodaemon = 0;
int from_inetd = 1;

int
main(int argc, char *argv[])
{
	SVCXPRT *transp;
	int s, salen;
	struct sockaddr_in sa;
        int sock = 0;
        int proto = 0;

	if (argc == 2 && !strcmp(argv[1], "-n"))
		nodaemon = 1;
	if (argc != 1 && !nodaemon) {
		printf("usage: %s [-n]\n", argv[0]);
		exit(1);
	}

	if (geteuid() == 0) {
		struct passwd *pep = getpwnam("nobody");
		if (pep)
			setuid(pep->pw_uid);
		else
			setuid(getuid());
	}

        /*
         * See if inetd started us
         */
        if (getsockname(0, (struct sockaddr *)&sa, &salen) < 0) {
                from_inetd = 0;
                sock = RPC_ANYSOCK;
                proto = IPPROTO_UDP;
        }
        
        if (!from_inetd) {
                if (!nodaemon)
                        possess();

                (void)pmap_unset(WALLPROG, WALLVERS);
                if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                        perror("socket");
                        exit(1);
                }
                bzero((char *)&sa, sizeof sa);
                if (bind(s, (struct sockaddr *)&sa, sizeof sa) < 0) {
                        perror("bind");
                        exit(1);
                }

                salen = sizeof sa;
                if (getsockname(s, (struct sockaddr *)&sa, &salen)) {
                        perror("getsockname");
                        exit(1);
                }

                pmap_set(WALLPROG, WALLVERS, IPPROTO_UDP, ntohs(sa.sin_port));
                if (dup2(s, 0) < 0) {
                        perror("dup2");
                        exit(1);
                }
                (void)pmap_unset(WALLPROG, WALLVERS);
        }

	(void)signal(SIGCHLD, killkids);

	transp = svcudp_create(sock);
	if (transp == NULL) {
		(void)fprintf(stderr, "cannot create udp service.\n");
		exit(1);
	}
	if (!svc_register(transp, WALLPROG, WALLVERS, wallprog_1, proto)) {
		(void)fprintf(stderr, "unable to register (WALLPROG, WALLVERS, udp).\n");
		exit(1);
	}
	svc_run();
	(void)fprintf(stderr, "svc_run returned\n");
	exit(1);

}

void possess(void)
{
	daemon(0, 0);
}

void killkids(int ignore)
{
	(void)ignore;
	while(wait4(-1, NULL, WNOHANG, NULL) > 0);
}

void *wallproc_wall_1(char **s, CLIENT *tmp1)
{
	(void)tmp1;

	/* fork, popen wall with special option, and send the message */
	if (fork() == 0) {
		FILE *pfp;

		pfp = popen(WALL_CMD, "w");
		if (pfp != NULL) {
			fprintf(pfp, "\007\007%s", *s);
			pclose(pfp);
			exit(0);
		}
	}
	return NULL;  /* ? */
}

void
wallprog_1(struct svc_req *rqstp, SVCXPRT *transp)
{
	union {
		char *wallproc_wall_1_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(XDR *, char **);
	xdrproc_t xdr_result;
	void *(*local)(char **, CLIENT *);

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void)svc_sendreply(transp, (xdrproc_t) xdr_void, NULL);
		goto leave;

	case WALLPROC_WALL:
		xdr_argument = xdr_wrapstring;
		xdr_result = (xdrproc_t) xdr_void;
		local = wallproc_wall_1;
		break;

	default:
		svcerr_noproc(transp);
		goto leave;
	}
	memset(&argument, 0, sizeof(argument));
	if (!svc_getargs(transp, xdr_argument, &argument)) {
		svcerr_decode(transp);
		goto leave;
	}
	result = (*local)((char **)&argument, (CLIENT *)rqstp);
	if (result != NULL && !svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, xdr_argument, &argument)) {
		(void)fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
leave:
        if (from_inetd)
                exit(0);
}
