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
 * From: @(#)process.c	5.10 (Berkeley) 2/26/91
 */
char rcsid[] = 
"$Id: process.c,v 1.14 1999/09/28 22:04:15 netbug Exp $";

/*
 * process.c handles the requests, which can be of three types:
 *	ANNOUNCE - announce to a user that a talk is wanted
 *	LEAVE_INVITE - insert the request into the table
 *	LOOK_UP - look up to see if a request is waiting in
 *		  in the table for the local user
 *	DELETE - delete invitation
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* #include <netinet/ip.h> <--- unused? */
#include <netdb.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
/* #include <paths.h> <---- unused? */
#include <utmp.h>
#include "prot_talkd.h"
#include "proto.h"

static int
check_one_utmp(const struct utmp *ut, const char *name)
{
	/* 
	 * We've got uptr->ut_name, and name, neither of which is
	 * necessarily null-terminated. Worse, either might be larger,
	 * depending on libc version. name came from a talkd
	 * structure, so it's at most NAME_SIZE.  ut->ut_name is at
	 * most UT_NAMESIZE. So the sum is guaranteed to hold
	 * either...
	 * Worse, strncmp isn't good enough. Suppose UT_NAMESIZE is 6 and 
	 * NAME_SIZE is 4, and ut_name is "abcdef" and name is "abcd".
	 * We don't really want those to match. Now, NAME_SIZE is 12, but
	 * UT_NAMESIZE is 256 on some platforms. I suppose maybe you *want*
	 * to be able to have name be a prefix of ut_name in this case, or
	 * you can't talk request people with huge names at all, but suppose
	 * there are several such people? Argh. *punt*
	 */
	char nametmp1[UT_NAMESIZE+NAME_SIZE+1];
	char nametmp2[UT_NAMESIZE+NAME_SIZE+1];

	strncpy(nametmp1, name, NAME_SIZE);
	strncpy(nametmp2, ut->ut_name, UT_NAMESIZE);
	nametmp1[NAME_SIZE] = 0;
	nametmp2[UT_NAMESIZE] = 0;

	if (strcmp(nametmp1, nametmp2)!=0) {
		debug("utmp: %s!=%s\n", nametmp1, nametmp2);
		return NOT_HERE;
	}
	debug("utmp: %s==%s\n", nametmp1, nametmp2);
	return SUCCESS;
}

static int
check_tty_perms(const char *tty, time_t *atime)
{
	struct stat statb;

	/* our current dir is /dev, no need to prepend it */
	if (stat(tty, &statb) != 0) {
		return FAILED;
	}
	if (!(statb.st_mode & 020)) {
		return PERMISSION_DENIED;
	}
	*atime = statb.st_atime;
	return SUCCESS;
}

/*
 * Search utmp for the local user
 */
static int
find_user(const char *name, char *tty)
{
	struct utmp *uptr;
	int found=0, ok=0, ret;
	time_t best_time = 0, this_time;
	char besttty[PATH_MAX];

	*besttty = 0;
	setutent();
	while ((uptr = getutent())!=NULL) {
#ifdef USER_PROCESS
		if (uptr->ut_type!=USER_PROCESS) continue;
#endif
		ret = check_one_utmp(uptr, name);
		if (ret!=SUCCESS) {
			/* wrong user */
			continue;
		}
		if (*tty && !strcmp(tty, uptr->ut_line)) {
			/* asked for a tty, found it */
			endutent();
			return SUCCESS;
		}
		ret = check_tty_perms(uptr->ut_line, &this_time);
		if (ret != SUCCESS) {
			found = 1; /* but don't set ok */
			continue;
		}
		found = ok = 1;
		if (this_time > best_time) {
			best_time = this_time;
			strcpy(besttty, uptr->ut_line);
		}
	}
	endutent();
	strcpy(tty, besttty);
	return !found ? NOT_HERE : (!ok ? PERMISSION_DENIED : SUCCESS);
}

static void
do_announce(CTL_MSG *mp, CTL_RESPONSE *rp, const char *fromhost)
{
	CTL_MSG *ptr;
	int result;
	u_int32_t temp;

	/* 
	 * See if the user is logged in.
	 * Clobbers mp->r_tty, replacing it with the tty to page them on.
	 * (If mp->r_tty is not empty, should only return that tty, or fail.)
	 */
	result = find_user(mp->r_name, mp->r_tty);
	if (result != SUCCESS) {
		rp->answer = result;
		return;
	}
	ptr = find_request(mp);
	if (ptr == NULL) {
		insert_table(mp, rp);
		rp->answer = announce(mp, fromhost);
		return;
	}

	/*
	 * Voodoo #1: if the client is opposite-endian and old/broken,
	 * it will have added one to the id while it was byte-swapped.
	 * Try undoing this. If it matches the id we have on file, it
	 * was a broken attempt to re-announce.
	 * We don't use ntohl because if we're big-endian ntohl is a nop.
	 */
	temp = byte_swap32(byte_swap32(mp->id_num)-1);

	if (mp->id_num > ptr->id_num || temp==ptr->id_num) {
		/*
		 * This is an explicit re-announce, so update the id_num
		 * field to avoid duplicates and re-announce the talk.
		 */
		ptr->id_num = new_id();
		rp->id_num = htonl(ptr->id_num);
		rp->answer = announce(mp, fromhost);
	} 
	else {
		/* duplicated or garbage request, so ignore it */
		rp->id_num = htonl(ptr->id_num);
		rp->answer = SUCCESS;
	}
}

void
process_request(CTL_MSG *mp, CTL_RESPONSE *rp, const char *fromhost)
{
	CTL_MSG *ptr;

	/* Ensure null-termination */
	mp->l_name[sizeof(mp->l_name)-1] = 0;
	mp->r_name[sizeof(mp->r_name)-1] = 0;
	mp->r_tty[sizeof(mp->r_tty)-1] = 0;
  
	rp->vers = TALK_VERSION;
	rp->type = mp->type;
	rp->id_num = htonl(0);
	if (mp->vers != TALK_VERSION) {
		syslog(LOG_WARNING, "Bad protocol version %d", mp->vers);
		rp->answer = BADVERSION;
		return;
	}
	mp->id_num = ntohl(mp->id_num);
	mp->pid = ntohl(mp->pid);
	print_request("process_request", mp);
  
	switch (mp->type) {
	  case ANNOUNCE:
		do_announce(mp, rp, fromhost);
		break;
	  case LEAVE_INVITE:
		ptr = find_request(mp);
		if (ptr != NULL) {
			rp->id_num = htonl(ptr->id_num);
			rp->answer = SUCCESS;
		} 
		else insert_table(mp, rp);
		break;
	  case LOOK_UP:
		ptr = find_match(mp);
		if (ptr != NULL) {
			rp->id_num = htonl(ptr->id_num);
			rp->addr = ptr->addr;
			rp->addr.ta_family = htons(ptr->addr.ta_family);
			rp->answer = SUCCESS;
		} 
		else rp->answer = NOT_HERE;
		break;
	  case DELETE:
		rp->answer = delete_invite(mp->id_num);
		break;
	  default:
		rp->answer = UNKNOWN_REQUEST;
		break;
	}
	print_response("process_request", rp);
}

