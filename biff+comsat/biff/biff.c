/*
 * Copyright (c) 1980 The Regents of the University of California.
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

char copyright[] =
"@(#) Copyright (c) 1980 The Regents of the University of California.\n"
"All rights reserved.\n";

/*
 * From: @(#)biff.c	5.3 (Berkeley) 6/1/90
 */
char rcsid[] = "$Id: biff.c,v 1.11 1999/12/12 13:19:27 dholland Exp $";

/*
 * biff
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../version.h"

int 
main(int argc, char *argv[])
{
	char *tty;       /* name of tty */
	struct stat stb; /* storage for return of stat on tty */
  
	/* 
	 * ttyname() returns a pointer to the pathname of the
	 * terminal device that is open on the specified file
	 * descriptor. If this fails tell the user.
	 */
  
	tty = ttyname(STDERR_FILENO);

	if (tty == NULL) {
		fprintf(stderr, "stderr is not a tty - where are you?\n");
		exit(1);
	}
  
	/* stat() the tty */
  
	if (stat(tty, &stb)==-1) {
		perror(tty);
		exit(1);
	}
  
	/* 
	 * If no command line arguments are specified then simply return
	 * the current status to the user.
	 */
  
	if (argc == 1)  {
		/* if user execute bit is on (ie biff y) */
		if(stb.st_mode&0100) {
			printf("is y\n");
		} else {
			printf("is n\n");
		}
	} else {
		/* If there's a command line argument... */
		/* switch based on argument */
		switch (argv[1][0]) {
		case 'y': 
			/* user entered biff y */
			if (chmod(tty, stb.st_mode|0100) == -1)
				perror(tty);
			break;

		case 'n': 
			/* user entered biff n */
			if (chmod(tty, stb.st_mode&~0100) == -1)
				perror(tty);
			break;
      
		default: 
			/* They used neither of the correct arguments -Doh! */
			fprintf(stderr, "usage: biff [y|n]\n");
		}
	}
	
	return (stb.st_mode&0100) ? 0 : 1; /* Return 0 if on or 1 if off */
}
