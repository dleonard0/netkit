/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Copyright 1999 David A. Holland. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notices, this list of conditions, and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notices, this list of conditions, and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgements:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 *	This product includes software developed by David A. Holland.
 * 4. Neither the name of the University nor the names of its contributors
 *    nor the names of other copyright holders may be used to endorse or 
 *    promote products derived from this software without specific prior 
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS, CONTRIBUTORS, AND OTHER AUTHORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS, CONTRIBUTORS, OR
 * OTHER AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * From: @(#)display.c	5.4 (Berkeley) 6/1/90
 *
 * Heavily modified 11/99 by dholland to add scrollback support.
 */
char display_rcsid[] = 
  "$Id: display.c,v 1.15 2000/07/29 18:50:27 dholland Exp $";

/*
 * The window 'manager', initializes curses and handles the actual
 * displaying of text
 */
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <curses.h>
#include "talk.h"

#define MAX_MAXLINES 16384

typedef struct {
	char *l_text;
} line;

typedef struct {
	line *s_lines;          /* Text in scroll buffer */
	int   s_nlines;         /* Number of valid lines in s_lines[] */
	int   s_maxlines;       /* Max already-allocated number of lines */
	int   s_scrollup;       /* Number of lines scrolled back */
	char *s_typebuf;        /* Current input line */
        int   s_typebufpos;     /* Current position in input line */
	int   s_typebufmax;     /* Max length of input line */
	char  cerase;           /* Erase-character key */
	char  werase;           /* Erase-word key */
	char  lerase;           /* Erase-line key */
} window;

static const char *topmessage;  /* Message at top of screen, if any */
static window my_win;           /* Scrolling data for top window */
static window his_win;          /* Scrolling data for bottom window */
static int sepline;             /* Line where separator is */
static int last_was_bot;        /* if 1, last win with activity was bottom */

static
void
do_sigwinch(int ignore)
{
	(void)ignore;

	/* Get curses to notice the size change */
	endwin();
	refresh();

	/* and now repaint. */
	dorefresh();
}

static
void
init_window(window *win)
{
	win->s_maxlines = 32;
	win->s_nlines = 0;
	win->s_lines = malloc(win->s_maxlines * sizeof(line));
	if (!win->s_lines) p_error("Out of memory");

	win->s_scrollup = 0;

	win->s_typebufmax = COLS+1;
	win->s_typebufpos = 0;
	win->s_typebuf = malloc(win->s_typebufmax);
	if (!win->s_typebuf) p_error("Out of memory");

	win->cerase = 2;   /* ^B */
	win->werase = 23;  /* ^W */
	win->lerase = 21;  /* ^U */
}

void
real_init_display(void)
{
	struct sigaction sigac;

	/* Open curses. */
	LINES = COLS = 0;
	if (initscr() == NULL) {
		printf("initscr failed: TERM is not set or set to an "
		       "unknown terminal type.\n");
		exit(1);
	}

	/* Block SIGALRM while handling SIGTSTP (curses catches SIGTSTP). */
	sigaction(SIGTSTP, NULL, &sigac);
	sigaddset(&sigac.sa_mask, SIGALRM);
	sigaction(SIGTSTP, &sigac, NULL);

	/* Grab SIGWINCH (curses cannot do this correctly on its own) */
	sigaction(SIGWINCH, NULL, &sigac);
	sigaddset(&sigac.sa_mask, SIGALRM);
	sigac.sa_handler = do_sigwinch;
	sigaction(SIGWINCH, &sigac, NULL);

	/* Set curses modes. */
	cbreak();
	noecho();
	nl();        /* force cr->lf */

	init_window(&my_win);
	init_window(&his_win);

	topmessage = NULL;
	sepline = LINES/2;

	/* Forcibly clear the screen. */
	clear();
	dorefresh();
}

void
set_my_edit_chars(int cerase, int lerase, int werase)
{
	my_win.cerase = cerase;
	my_win.lerase = lerase;
	my_win.werase = werase;
}

void
set_his_edit_chars(int cerase, int lerase, int werase)
{
	his_win.cerase = cerase;
	his_win.lerase = lerase;
	his_win.werase = werase;
}

/**************************************************************/

void 
dobeep(void)
{
	beep();
}

static
void
refresh_window(window *win, int nlines, int typeline)
{
	int i, line, topline = typeline-nlines+1;
	for (i=nlines-1; i>=0; i--) {
		move(topline+i, 0);
		line = win->s_nlines - (nlines - 1) + i - win->s_scrollup;
		if (line >=0 && line < win->s_nlines) {
			addstr(win->s_lines[line].l_text);
		}
		else if (line==win->s_nlines) {
			addstr(win->s_typebuf);
		}
		clrtoeol();
	}
}

void
dorefresh(void)
{
	int i;
	erase();

	sepline = LINES/2;

	/* Separator */
	move(sepline, 0);
	for (i=0; i<COLS; i++) addch(ACS_HLINE);

	/* Top window */
	refresh_window(&my_win, sepline, sepline-1);
	
	/* Bottom window */
	refresh_window(&his_win, LINES-sepline-1, LINES-1);

	/* Message */
	if (topmessage) {
		move(0, 0);
		addch('[');
		addstr(topmessage);
		addch(']');
	}

	/* Scroll indicators */
	if (my_win.s_scrollup) {
		move(sepline, 4);
		addstr("[upper scroll]");
	}
	if (his_win.s_scrollup) {
		move(sepline, COLS-4-14);
		addstr("[lower scroll]");
	}

	if (last_was_bot) {
		if (his_win.s_scrollup) {
			move(sepline, COLS-5);
		}
		else {
			move(LINES-1, his_win.s_typebufpos);
		}
	}
	else {
		if (my_win.s_scrollup) {
			move(sepline, 17);
		}
		else {
			move(sepline-1, my_win.s_typebufpos);
		}
	}

	refresh();
}

/**************************************************************/

static
void
doscroll(window *win, int delta)
{
	int nsc = win->s_scrollup + delta;
	if (nsc<0) nsc = 0;
	if (nsc>win->s_nlines) nsc = win->s_nlines;
	win->s_scrollup = nsc;
	dorefresh();
}

static
int
do_one_getch(void)
{
	static int gotesc = 0;
	int ich;
	unsigned char ch;

	ich = getch();
	if (ich==ERR) {
		return -1;
	}

	ch = (unsigned char)ich;

	if (!gotesc && ch==27) {
		gotesc = 1;
		return -1;
	}
	if (gotesc) {
		gotesc = 0;
		return ((int)ch)|256;
	}
#if 0 /* blah - someone please fix this */
	if (ch & 128) {
		/*
		 * It would be nice to be able to tell if this is meant to 
		 * be a meta-modifier, in which case we should flip it to
		 * the next bit over, or an actual 8-bit character, in
		 * which case it should be passed through.
		 *
		 * The following kludge probably won't work right. When will
		 * we get *working* international charset support in unix?
		 * XXX.
		 */
		const char *foo = getenv("LC_CTYPE");
		if (!foo) {
			return = ((int)(ch&127))|256;
		}
	}
#endif
	return ch;
}

/*
 * Note: at this level we trap scrolling keys and other local phenomena.
 * Erase/word erase/line erase, newlines, and so forth get passed through,
 * sent to the other guy, and ultimately come out in display().
 */
int
dogetch(void)
{
	int k = do_one_getch();
	int scrl = sepline-2;  /* number of lines to scroll by */

	if (k==('p'|256)) {          /* M-p: scroll our window up */
		doscroll(&my_win, scrl);
	}
	else if (k==('n'|256)) {     /* M-n: scroll our window down */
		doscroll(&my_win, -scrl);
	}
	else if (k==('p'&31)) {      /* C-p: scroll other window up */
		doscroll(&his_win, scrl);
	}
	else if (k==('n'&31)) {      /* C-n: scroll other window down */
		doscroll(&his_win, -scrl);
	}
	else if (k == '\f') {        /* C-l: reprint */
		clear();
		dorefresh();
	}
	else if (k>=0) {
		return k;
	}
	return -1;
}

/**************************************************************/

static
void
display_lerase(window *win)
{
	win->s_typebuf[0] = 0;
	win->s_typebufpos = 0;
}

static
void
discard_top_line(window *win)
{
	int i;
	assert(win->s_nlines>0);
	free(win->s_lines[0].l_text);
	for (i=0; i<win->s_nlines-1; i++) {
		win->s_lines[i] = win->s_lines[i+1];
	}
	win->s_nlines--;
}

static
void
display_eol(window *win)
{
	line *tmpl;
	char *tmps;

	if (win->s_nlines == win->s_maxlines) {
		if (win->s_maxlines < MAX_MAXLINES) {
			win->s_maxlines *= 2;
			tmpl = realloc(win->s_lines, 
				       win->s_maxlines*sizeof(line));
		}
		else {
			/* Reached size limit - pretend realloc failed */
			tmpl = NULL;
		}

		if (!tmpl) {
			discard_top_line(win);
		}
		else {
			win->s_lines = tmpl;
		}
	}
	assert(win->s_nlines < win->s_maxlines);
	
	while ((tmps = strdup(win->s_typebuf))==NULL && win->s_nlines>0) {
		discard_top_line(win);
	}
	if (!tmps) {
		p_error("Out of memory");
	}

	win->s_lines[win->s_nlines++].l_text = tmps;

	display_lerase(win);

	if (win==&my_win) topmessage = NULL;
}

static
void
display_addch(window *win, int ch)
{
	/*
	 * Leave one extra byte of space in the type buffer. This is so that
	 * the last column of the screen doesn't get used, because the refresh
	 * code does clreol after it, and that clears the next line of the 
	 * screen, which makes a mess.
	 */
	if (win->s_typebufpos+2 == win->s_typebufmax) {
		display_eol(win);
	}
	win->s_typebuf[win->s_typebufpos++] = ch;
	win->s_typebuf[win->s_typebufpos] = 0;
}

static
void
display_tab(window *win) {
	while (win->s_typebufpos%8 != 0) {
		display_addch(win, ' ');
	}
}

static
void
display_cerase(window *win)
{
	if (win->s_typebufpos > 0) {
		win->s_typebuf[--win->s_typebufpos] = 0;
	}
}

static
void
display_werase(window *win)
{
	/*
	 * Search backwards until we find the beginning of a word or 
	 * the beginning of the line.
	 */
	int lastpos=win->s_typebufpos;
	int pos = lastpos;

	while (pos>=0) {
		int onword = pos<lastpos && win->s_typebuf[pos]!=' ';
		int prevspace = pos==0 || win->s_typebuf[pos-1]==' ';
		if (onword && prevspace) break;
		pos--;
	}
	if (pos<0) pos = 0;

	win->s_typebuf[pos] = 0;
	win->s_typebufpos = pos;
}

/*
 * Display some text on somebody's window, processing some control
 * characters while we are at it.
 */
void
display(int hiswin, unsigned char *text, int size)
{
	int j;
	window *win = hiswin ? &his_win : &my_win;
	last_was_bot = hiswin;

	for (j = 0; j < size; j++) {
		if (text[j] == '\n' || text[j]=='\r') {
			display_eol(win);
		}
		else if (text[j]=='\b' || 
			 text[j]==127 || 
			 text[j]==win->cerase) {

			/* someday erase characters will work right in unix */
			display_cerase(win);
		}
		else if (text[j] == win->werase) {
			display_werase(win);
		}
		else if (text[j] == win->lerase) {
			/* line kill */
			display_lerase(win);
		}
		else if (text[j] == '\a') {
			beep();
		}
		else if (text[j] == '\f') {
			/* nothing */
		}
		else if (text[j] == '\t') {
			display_tab(win);
		}
		else if ((text[j] & 0x7F) < ' ') {
			display_addch(win, '^');
			display_addch(win, (text[j] & 63) + 64);
		} 
		else {
			display_addch(win, text[j]);
		}
	}
	dorefresh();
}

/**************************************************************/

/*
 * Display string in the standard location
 */
void
message(const char *string)
{
	topmessage = string;
	dorefresh();
}

static
int
check_alt_screen(void)
{
	const char *rmcup, *smcup;
	rmcup = tigetstr("rmcup");
	smcup = tigetstr("smcup");
	return rmcup || smcup;
}

static
void
middle_message(const char *string, const char *string2)
{
	int line = sepline;
	int width, start;

	dorefresh();

	width = 2 + strlen(string) + strlen(string2) + 2;
	start = (COLS - width)/2;
	if (start<0) start = 0;

	move(line, start);
	addstr("[ ");
	addstr(string);
	addstr(string2);
	addstr(" ]");
	refresh();
}

static
void
do_quit(void)
{
	move(LINES-1, 0);
	clrtoeol();
	refresh();
	endwin();

	if (invitation_waiting) {
		send_delete();
	}
	exit(0);
}

/*
 * p_error prints the system error message on the standard location
 * on the screen and then exits. (i.e. a curses version of perror)
 */
void
p_error(const char *string) 
{
	char msgbuf[256];
	int hold = check_alt_screen();

	snprintf(msgbuf, sizeof(msgbuf), "%s: %s", string, strerror(errno));
	middle_message(msgbuf, hold ? ". Press any key..." : "");

	if (hold) {
		/* alternative screen, wait before exiting */
		getch();
	}

	do_quit();
}

/*
 * All done talking...hang up the phone and reset terminal thingy
 */
void
quit(int direct)
{
	int hold = check_alt_screen();

	if (direct != 1 && hold) {
		/* alternative screen, prompt before exit */
		middle_message("Press any key to exit", "");
		getch();
	}
	else {
		/* Make sure any message appears */
		dorefresh();
	}

	do_quit();
}
