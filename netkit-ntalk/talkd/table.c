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
 * From: @(#)table.c	5.7 (Berkeley) 2/26/91
 */
char table_rcsid[] = 
  "$Id: table.c,v 1.9 1998/11/27 07:58:47 dholland Exp $";

/*
 * Routines to handle insertion, deletion, etc on the table
 * of requests kept by the daemon. Nothing fancy here, linear
 * search on a double-linked list. A time is kept with each 
 * entry so that overly old invitations can be eliminated.
 *
 * Consider this a mis-guided attempt at modularity
 */
#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "prot_talkd.h"
#include "proto.h"

#define MAX_ID 16000	/* << 2^15 so I don't have sign troubles */

typedef struct table_entry {
	struct table_entry *next;
	struct table_entry *last;
	CTL_MSG request;
	time_t time;
} TABLE_ENTRY;

static TABLE_ENTRY *table = NULL;

/*
 * Generate a unique non-zero sequence number
 */
int
new_id(void)
{
	static int current_id = 0;

	current_id = (current_id + 1) % MAX_ID;
	/* 0 is reserved, helps to pick up bugs */
	if (current_id == 0)
		current_id = 1;
	return current_id;
}

/*
 * Classic delete from a double-linked list
 */
static void
deleteit(TABLE_ENTRY *ptr)
{
	print_request("deleteit", &ptr->request);
	if (table == ptr) {
		table = ptr->next;
	}
	else if (ptr->last != NULL) {
		ptr->last->next = ptr->next;
	}
	if (ptr->next != NULL) {
		ptr->next->last = ptr->last;
	}
	free(ptr);
}

/*
 * Go through the table and chuck out anything out of date
 */
static void
expire(void)
{
	time_t current_time = time(NULL);
	TABLE_ENTRY *ptr;

	for (ptr = table; ptr != NULL; ptr = ptr->next) {
		if ((current_time - ptr->time) > MAX_LIFE) {
			/* the entry is too old */
			print_request("deleting expired entry",
				      &ptr->request);
			deleteit(ptr);
		}
	}
}

/*
 * Look in the table for an invitation that matches the current
 * request looking for an invitation
 */
CTL_MSG *
find_match(CTL_MSG *request)
{
	TABLE_ENTRY *ptr;

	expire();

	print_request("find_match", request);
	for (ptr = table; ptr != NULL; ptr = ptr->next) {
		print_request("", &ptr->request);
		if (strcmp(request->l_name, ptr->request.r_name) == 0 &&
		    strcmp(request->r_name, ptr->request.l_name) == 0 &&
		     ptr->request.type == LEAVE_INVITE)
			return (&ptr->request);
	}
	return NULL;
}

/*
 * Look for an identical request, as opposed to a complementary
 * one as find_match does 
 */
CTL_MSG *
find_request(CTL_MSG *request)
{
	TABLE_ENTRY *ptr;

	expire();

	/*
	 * See if this is a repeated message.
	 */
	print_request("find_request", request);
	for (ptr = table; ptr != NULL; ptr = ptr->next) {
		print_request("", &ptr->request);
		if (strcmp(request->r_name, ptr->request.r_name) == 0 &&
		    strcmp(request->l_name, ptr->request.l_name) == 0 &&
		    request->type == ptr->request.type &&
		    request->pid == ptr->request.pid) {
			/* update the time if we 'touch' it */
			ptr->time = time(NULL);
			return (&ptr->request);
		}
	}
	return NULL;
}

void
insert_table(CTL_MSG *request, CTL_RESPONSE *response)
{
	TABLE_ENTRY *ptr;

	request->id_num = new_id();
	response->id_num = htonl(request->id_num);

	/* insert a new entry into the top of the list */
	ptr = malloc(sizeof(TABLE_ENTRY));
	if (ptr == NULL) {
		syslog(LOG_ERR, "insert_table: Out of memory");
		_exit(1);
	}
	ptr->time = time(NULL);
	ptr->request = *request;

	ptr->next = table;
	if (ptr->next != NULL) {
		ptr->next->last = ptr;
	}
	ptr->last = NULL;
	table = ptr;
}

/*
 * Delete the invitation with id 'id_num'
 */
int
delete_invite(unsigned id_num)
{
	TABLE_ENTRY *ptr;

	ptr = table;
	debug("delete_invite(%d)\n", id_num);

	for (ptr = table; ptr != NULL; ptr = ptr->next) {
		if (ptr->request.id_num == id_num)
			break;
		print_request("", &ptr->request);
	}
	if (ptr != NULL) {
		deleteit(ptr);
		return SUCCESS;
	}
	return NOT_HERE;
}

