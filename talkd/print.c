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
 * From: @(#)print.c	5.8 (Berkeley) 2/26/91
 */
char print_rcsid[] = 
  "$Id: print.c,v 1.3 1996/08/15 03:44:13 dholland Exp $";

/* debug print routines */

#include <sys/types.h>
#include <sys/socket.h>
#include <protocols/talkd.h>
#include <syslog.h>
#include <stdio.h>
#include "proto.h"

static const char *types[] = { 
  "leave_invite", 
  "look_up", 
  "delete", 
  "announce" 
};
#define	NTYPES	(sizeof (types) / sizeof (types[0]))

static const char *answers[] = { 
  "success", 
  "not_here", 
  "failed", 
  "machine_unknown", 
  "permission_denied", 
  "unknown_request", 
  "badversion", 
  "badaddr", 
  "badctladdr" 
};
#define	NANSWERS	(sizeof (answers) / sizeof (answers[0]))

void
print_request(const char *cp, const CTL_MSG *mp)
{
	char tbuf[80];
	const char *tp;
	
	if (mp->type > NTYPES) {
		snprintf(tbuf, sizeof(tbuf), "type %d", mp->type);
		tp = tbuf;
	} 
	else tp = types[mp->type];

	syslog(LOG_DEBUG, "%s: %s: id %d, l_user %s, r_user %s, r_tty %s",
	    cp, tp, mp->id_num, mp->l_name, mp->r_name, mp->r_tty);
}

void
print_response(const char *cp, const CTL_RESPONSE *rp)
{
	char tbuf[80], abuf[80];
	const char *tp, *ap;
	
	if (rp->type > NTYPES) {
		snprintf(tbuf, sizeof(tbuf), "type %d", rp->type);
		tp = tbuf;
	} 
	else tp = types[rp->type];

	if (rp->answer > NANSWERS) {
		snprintf(abuf, sizeof(abuf), "answer %d", rp->answer);
		ap = abuf;
	} 
	else ap = answers[rp->answer];

	syslog(LOG_DEBUG, "%s: %s: %s, id %d", cp, tp, ap, ntohl(rp->id_num));
}
