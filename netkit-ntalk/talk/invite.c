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
 * From: @(#)invite.c	5.8 (Berkeley) 3/1/91
 */
char inv_rcsid[] = 
  "$Id: invite.c,v 1.12 1999/09/28 22:04:15 netbug Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
#include <netinet/in.h>
#include <errno.h>
#include <setjmp.h>
#include <unistd.h>
#include "talk.h"

static void announce_invite(void);

/*
 * There wasn't an invitation waiting, so send a request containing
 * our sockt address to the remote talk daemon so it can invite
 * him 
 */

/*
 * The msg.id's for the invitations
 * on the local and remote machines.
 * These are used to delete the 
 * invitations.
 */
int	local_id, remote_id;
void	re_invite(int);
sigjmp_buf invitebuf;

void
invite_remote(void)
{
	/*volatile int nfd, read_mask, template;*/
	volatile int new_sockt;
	struct itimerval itimer;
	CTL_RESPONSE response;
	struct sockaddr_in sn;
	socklen_t size = sizeof(sn);

	unsigned long int endvalue=-1;
	
	itimer.it_value.tv_sec = RING_WAIT;
	itimer.it_value.tv_usec = 0;
	itimer.it_interval = itimer.it_value;
	if (listen(sockt, 5) != 0) {
		p_error("Error on attempt to listen for caller");
	}

	getsockname(sockt, (struct sockaddr *)&sn, &size);

	/* copy new style sockaddr to old */
	msg.addr.ta_family = htons(AF_INET);
	msg.addr.ta_port = sn.sin_port;
	msg.addr.ta_addr = 0;  /* will be patched up in ctl_transact */

	msg.id_num = htonl(endvalue);		/* an impossible id_num */
	invitation_waiting = 1;
	announce_invite();
	/*
	 * Shut off the automatic messages for a while,
	 * so we can use the interupt timer to resend the invitation
	 */
	end_msgs();
	setitimer(ITIMER_REAL, &itimer, (struct itimerval *)0);
	message("Waiting for your party to respond");
	signal(SIGALRM, re_invite);
	(void) sigsetjmp(invitebuf, 1);
	while ((new_sockt = accept(sockt, 0, 0)) < 0) {
		if (errno == EINTR)
			continue;
		p_error("Unable to connect with your party");
	}
	close(sockt);
	sockt = new_sockt;

	/*
	 * Have the daemons delete the invitations now that we
	 * have connected.
	 */
	current_state = "Waiting for your party to respond";
	start_msgs();

	msg.id_num = htonl(local_id);
	ctl_transact(MY_DAEMON, msg, DELETE, &response);
	msg.id_num = htonl(remote_id);
	ctl_transact(HIS_DAEMON, msg, DELETE, &response);
	invitation_waiting = 0;
}

/*
 * Routine called on interupt to re-invite the callee
 */
void
re_invite(int ignore)
{
	(void)ignore;
	signal(SIGALRM, re_invite);

	message("Ringing your party again");
	current_line++;
	/* force a re-announce */
	msg.id_num = htonl(remote_id + 1);
	announce_invite();
	siglongjmp(invitebuf, 1);
}

static const char *answers[] = {
	"answer #0",					/* SUCCESS */
	"Your party is not logged on",			/* NOT_HERE */
	"Target machine is too confused to talk to us",	/* FAILED */
	"Target machine does not recognize us",		/* MACHINE_UNKNOWN */
	"Your party is refusing messages",		/* PERMISSION_REFUSED */
	"Target machine can not handle remote talk",	/* UNKNOWN_REQUEST */
	"Target machine indicates protocol mismatch",	/* BADVERSION */
	"Target machine indicates protocol botch (addr)",/* BADADDR */
	"Target machine indicates protocol botch (ctl_addr)",/* BADCTLADDR */
};
#define	NANSWERS	(sizeof (answers) / sizeof (answers[0]))

/*
 * Transmit the invitation and process the response
 */
static void
announce_invite(void)
{
	CTL_RESPONSE response;

	current_state = "Trying to connect to your party's talk daemon";
	ctl_transact(HIS_DAEMON, msg, ANNOUNCE, &response);
	remote_id = response.id_num;
	if (response.answer != SUCCESS) {
		if (response.answer < NANSWERS)
			message(answers[response.answer]);
		quit(0);
	}
	/* leave the actual invitation on my talk daemon */
	ctl_transact(MY_DAEMON, msg, LEAVE_INVITE, &response);
	local_id = response.id_num;
}

/*
 * Tell the daemon to remove your invitation
 */
void
send_delete(void)
{
	/*
	 * This is just a extra clean up, so just send it
	 * and don't wait for an answer
	 */
	send_one_delete(HIS_DAEMON, remote_id);
	send_one_delete(MY_DAEMON, local_id);
}
