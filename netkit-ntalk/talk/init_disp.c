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
 * From: @(#)init_disp.c	5.4 (Berkeley) 6/1/90
 */
char id_rcsid[] = 
  "$Id: init_disp.c,v 1.12 1999/11/25 04:23:38 dholland Exp $";

/*
 * Initialization code for the display package,
 * as well as the signal handling routines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>

#include "talk.h"

static void sig_sent(int);

/* 
 * Set up curses, catch the appropriate signals,
 * and build the various windows.
 */
void
init_display(void)
{
	real_init_display();

	signal(SIGINT, sig_sent);
	signal(SIGPIPE, sig_sent);

	/* let them know we are working on it */
	current_state = "No connection yet";
}

/*
 * Trade edit characters with the other talk. By agreement
 * the first three characters each talk transmits after
 * connection are the three edit characters.
 */
void
set_edit_chars(void)
{
	char buf[3];
	int cc;
	struct termios tios;
	int cerase, lerase, werase;

	tcgetattr(0, &tios);

	cerase = tios.c_cc[VERASE];
	lerase = tios.c_cc[VKILL];
	werase = tios.c_cc[VWERASE];

	if ((unsigned char)werase == 0xff) {
		werase = '\027';	 /* control W */
	}

	set_my_edit_chars(cerase, lerase, werase);

	buf[0] = cerase;
	buf[1] = lerase;
	buf[2] = werase;
	cc = write(sockt, buf, sizeof(buf));
	if (cc != sizeof(buf) )
		p_error("Lost the connection");
	cc = read(sockt, buf, sizeof(buf));
	if (cc != sizeof(buf) )
		p_error("Lost the connection");

	cerase = buf[0];
	lerase = buf[1];
	werase = buf[2];

	set_his_edit_chars(cerase, lerase, werase);
}

void
sig_sent(int signum)
{
	(void)signum;
	message("Connection closing. Exiting");
	quit(1);
}

