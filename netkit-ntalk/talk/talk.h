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
 *
 *	from: @(#)talk.h	5.7 (Berkeley) 3/1/91
 *	$Id: talk.h,v 1.15 1999/11/25 07:46:44 dholland Exp $
 */

/*#include <curses.h>*/
#include <sys/socket.h>
#include <netinet/in.h>

#include "prot_talkd.h"

extern int sockt;
extern int invitation_waiting;

extern const char *current_state;
extern int current_line;

void p_error(const char *string);
void quit(int);
void message(const char *mesg);
void get_names(int argc, char *argv[]);
void get_addrs(const char *);
void init_display(void);
void real_init_display(void);
void open_ctl(void);
void open_sockt(void);
void start_msgs(void);
int check_local(void);
void invite_remote(void);
void end_msgs(void);
void set_edit_chars(void);
void talk(void);
void send_delete(void);
void display(int hiswin, unsigned char *, int);

void set_my_edit_chars(int ctrlh, int ctrlu, int ctrlw);
void set_his_edit_chars(int ctrlh, int ctrlu, int ctrlw);
void dobeep(void);
void dorefresh(void);
int dogetch(void);  /* returns 0-255 or -1 meaning no character */

#define HIS_DAEMON 0
#define MY_DAEMON 1
void send_one_delete(int ismydaemon, int);
void ctl_transact(int ismydaemon, CTL_MSG, int, CTL_RESPONSE *);

extern	struct in_addr his_machine_addr;
extern	u_short daemon_port;
extern	CTL_MSG msg;
