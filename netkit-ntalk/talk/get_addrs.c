/*
 * Copyright (c) 1983 Regents of the University of California.
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

/*
 * From: @(#)get_addrs.c	5.7 (Berkeley) 3/1/91
 */
char ga_rcsid[] = 
  "$Id: get_addrs.c,v 1.11 1999/09/28 22:04:14 netbug Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "talk.h"

void
get_addrs(const char *his_machine_name)
{
	struct hostent *hp;
	struct servent *sp;

	msg.pid = htonl(getpid());

	hp = gethostbyname(his_machine_name);
	if (hp == NULL) {
		fprintf(stderr, "talk: %s: ", his_machine_name);
		herror((char *)NULL);
		exit(-1);
	}
	if (hp->h_length > (int)sizeof(his_machine_addr)) {
		hp->h_length = sizeof(his_machine_addr);
	}
	memcpy(&his_machine_addr, hp->h_addr, hp->h_length);

	/* find the server's port */
	sp = getservbyname("ntalk", "udp");
	if (sp == 0) {
		fprintf(stderr, "talk: %s/%s: service is not registered.\n",
		     "ntalk", "udp");
		exit(-1);
	}
	daemon_port = sp->s_port;
}
