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

char rusersd_rcsid[] = 
  "$Id: rusersd.c,v 1.10 2000/07/23 04:09:28 dholland Exp $";

#include <stdio.h>
#include <signal.h>
#include <sys/socket.h>
#include <syslog.h>
#include <pwd.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <unistd.h>
#include <grp.h>
#ifdef __GLIBC__
	#include <rpcsvc/rusers.h>
#else
	#include "rusers.h"
#endif

#include "../version.h"

void rusers_service(struct svc_req *rqstp, SVCXPRT *transp);
int daemon(int, int);

int from_inetd = 1;

static
void
cleanup(int ignore)
{
	(void)ignore;

        pmap_unset(RUSERSPROG, RUSERSVERS_3);
        pmap_unset(RUSERSPROG, RUSERSVERS_IDLE);
        exit(0);
}

int
main(void)
{
	SVCXPRT *transp;
        int sock = 0;
        int proto = 0;
	struct sockaddr_in from;
	int fromlen = sizeof(from);

 
	/* Open syslog */
	openlog("rpc.rusersd", LOG_PID, LOG_DAEMON);
 
	/* Drop privilege */
	if (getuid() == 0) {
		struct passwd	*pw;
 
		if ((pw = getpwnam("nobody")) == NULL) {
			syslog(LOG_WARNING, "Unable to find user nobody: %m");
			exit(1);
		}
		if (setgroups(1, &pw->pw_gid) < 0
		 || setgid(pw->pw_gid) < 0
		 || setuid(pw->pw_uid) < 0) {
			syslog(LOG_WARNING, "Failed to drop privilege: %m");
			exit(1);
		}
	}
 
	
	/*
	 * See if inetd started us
	 */
	if (getsockname(0, (struct sockaddr *)&from, &fromlen) < 0) {
		from_inetd = 0;
		sock = RPC_ANYSOCK;
		proto = IPPROTO_UDP;
	}
	
	if (!from_inetd) {
		daemon(0, 0);

		pmap_unset(RUSERSPROG, RUSERSVERS_3);
		pmap_unset(RUSERSPROG, RUSERSVERS_IDLE);

		signal(SIGINT, cleanup);
		signal(SIGTERM, cleanup);
		signal(SIGHUP, cleanup);
	}

	openlog("rpc.rusersd", LOG_PID, LOG_DAEMON);
	
	transp = svcudp_create(sock);
	if (transp == NULL) {
		syslog(LOG_ERR, "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, RUSERSPROG, RUSERSVERS_3, rusers_service, proto)) {
		syslog(LOG_ERR, "unable to register (RUSERSPROG, RUSERSVERS_3, %s).", proto?"udp":"(inetd)");
		exit(1);
	}

	if (!svc_register(transp, RUSERSPROG, RUSERSVERS_IDLE, rusers_service, proto)) {
		syslog(LOG_ERR, "unable to register (RUSERSPROG, RUSERSVERS_IDLE, %s).", proto?"udp":"(inetd)");
		exit(1);
	}

	svc_run();
	syslog(LOG_ERR, "svc_run returned");
	exit(1);
}
