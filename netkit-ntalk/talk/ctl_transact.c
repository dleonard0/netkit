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
 * From: @(#)ctl_transact.c	5.8 (Berkeley) 3/1/91
 */
char ctlt_rcsid[] = 
  "$Id: ctl_transact.c,v 1.12 1999/09/28 22:04:14 netbug Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
/* #include <netinet/ip.h> looks like this is not needed (no functions used) */
#include <string.h>
#include <errno.h>
#include "talk.h"

#define CTL_WAIT 2	/* time to wait for a response, in seconds */

/* We now make two UDP sockets, one for the local talkd, one for the remote. */
static int to_local_talkd;
static int to_remote_talkd;
static u_int32_t local_addr_for_remote;


/* open the ctl sockets */
void
open_ctl(void) 
{
	struct sockaddr_in loc, rem;
	socklen_t length;
	int on=1;

	to_local_talkd = socket(AF_INET, SOCK_DGRAM, 0);
	to_remote_talkd = socket(AF_INET, SOCK_DGRAM, 0);

	if (to_local_talkd < 0 || to_remote_talkd < 0) {
		p_error("Bad socket");
	}

#ifdef SO_BSDCOMPAT
	/* 
	 * Linux does some async error return stuff that
	 * really disagrees with us. So we disable it.
	 */
	setsockopt(to_local_talkd, SOL_SOCKET, SO_BSDCOMPAT, &on, sizeof(on));
	setsockopt(to_remote_talkd, SOL_SOCKET, SO_BSDCOMPAT, &on, sizeof(on));
#endif

	/*
	 * Explicitly talk to the local daemon over loopback.
	 */
	memset(&loc, 0, sizeof(loc));
	loc.sin_family = AF_INET;
	loc.sin_port = htons(0);
	loc.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (bind(to_local_talkd, (struct sockaddr *)&loc, sizeof(loc))<0) {
		p_error("Couldn't bind local control socket");
	}

	memset(&loc, 0, sizeof(loc));
	loc.sin_family = AF_INET;
	loc.sin_port = daemon_port;
	loc.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (connect(to_local_talkd, (struct sockaddr *)&loc, sizeof(loc))<0) {
		p_error("Couldn't connect local control socket");
	}

	/*
	 * Now the trick - don't bind the remote socket. Instead
	 * just do a UDP connect on it to force it to talk to the 
	 * remote talkd. The neat side effect of this is that
	 * we can then get the correct local IP back from it
	 * with getsockname.
	 */
	memset(&rem, 0, sizeof(rem));
	rem.sin_family = AF_INET;
	rem.sin_port = daemon_port;
	rem.sin_addr = his_machine_addr;
	if (connect(to_remote_talkd, (struct sockaddr *)&rem, sizeof(rem))<0) {
		p_error("Couldn't connect remote control socket");
	}

	length = sizeof(rem);
	if (getsockname(to_remote_talkd, (struct sockaddr *)&rem, &length)<0) {
		p_error("getsockname");
	}
	local_addr_for_remote = rem.sin_addr.s_addr;
}

static void
send_packet(CTL_MSG *msg, int sock)
{
	int cc;
	cc = send(sock, msg, sizeof(*msg), 0);
	if (cc<0 && errno == EINTR) {
		return;
	}
	else if (cc<0) {
	    p_error("Error on write to talk daemon");
	}
	else if (cc != sizeof(*msg)) {
	    p_error("Short write to talk daemon");
	}
}

static void
clean_up_packet(int sock, CTL_MSG *msg, int type)
{
	struct sockaddr_in here;
	socklen_t len = sizeof(here);

	msg->vers = TALK_VERSION;
	msg->type = type;
	msg->answer = 0;
	msg->pad = 0;

	if (getsockname(sock, (struct sockaddr *)&here, &len)<0) {
	    p_error("getsockname");
	}
	msg->ctl_addr.ta_family = htons(AF_INET);
	msg->ctl_addr.ta_port = here.sin_port;
	msg->ctl_addr.ta_addr = here.sin_addr.s_addr;
	msg->addr.ta_family = htons(AF_INET);
	msg->addr.ta_addr = local_addr_for_remote;
}

void
send_one_delete(int ismydaemon, int id)
{
	CTL_MSG tmp = msg;
	int sock = ismydaemon ? to_local_talkd : to_remote_talkd;

	tmp.id_num = htonl(id);
	clean_up_packet(sock, &tmp, DELETE);
	send_packet(&tmp, sock);
}

/*
 * SOCKDGRAM is unreliable, so we must repeat messages if we have
 * not received an acknowledgement within a reasonable amount
 * of time
 */
static void
do_transact(int sock, CTL_MSG *mesg, CTL_RESPONSE *rp)
{
	fd_set read_mask, ctl_mask;
	int nready=0, cc;
	struct timeval wait;

	FD_ZERO(&ctl_mask);
	FD_SET(sock, &ctl_mask);

	/*
	 * Keep sending the message until a response of
	 * the proper type is obtained.
	 */
	do {
		/* resend message until a response is obtained */
		do {
			send_packet(mesg, sock);
			read_mask = ctl_mask;
			wait.tv_sec = CTL_WAIT;
			wait.tv_usec = 0;
			nready = select(sock+1, &read_mask, 0, 0, &wait);
			if (nready < 0) {
				if (errno == EINTR)
					continue;
				p_error("Error waiting for daemon response");
			}
		} while (nready == 0);
		/*
		 * Keep reading while there are queued messages 
		 * (this is not necessary, it just saves extra
		 * request/acknowledgements being sent)
		 */
		do {
			cc = recv(sock, (char *)rp, sizeof (*rp), 0);
			if (cc < 0) {
				if (errno == EINTR)
					continue;
				p_error("Error on read from talk daemon");
			}
			read_mask = ctl_mask;
			/* an immediate poll */
			timerclear(&wait);
			nready = select(sock+1, &read_mask, 0, 0, &wait);
		} while (nready > 0 && (rp->vers != TALK_VERSION ||
		    rp->type != mesg->type));
	} while (rp->vers != TALK_VERSION || rp->type != mesg->type);
}

void
ctl_transact(int ismydaemon, CTL_MSG mesg, int type, CTL_RESPONSE *rp)
{
	int sock;

	sock = ismydaemon ? to_local_talkd : to_remote_talkd;

	clean_up_packet(sock, &mesg, type);

	do_transact(sock, &mesg, rp);

	rp->id_num = ntohl(rp->id_num);
	rp->addr.ta_family = ntohs(rp->addr.ta_family);
}

