/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Tony Nardo of the Johns Hopkins University/Applied Physics Lab.
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

#ifndef lint
/*static char sccsid[] = "from: @(#)net.c	5.5 (Berkeley) 6/1/90";*/
char net_rcsid[] = "$Id: net.c,v 1.7 1997/02/01 22:26:56 dholland Exp $";
#endif /* not lint */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "finger.h"

void netfinger(const char *name) {
	register FILE *fp;
	struct in_addr defaddr;
	register int c, lastc = 0;
	struct hostent *hp, def;
	struct servent *sp;
	struct sockaddr_in sn;
	int s;
	char *alist[1], *host;

	host = strrchr(name, '@');
	if (!host) return;

	*host++ = '\0';
	hp = gethostbyname(host);
	if (!hp) {
		if (!inet_aton(host, &defaddr)) {
			fprintf(stderr, "finger: unknown host: %s\n", host);
			return;
		}
		def.h_name = host;
		def.h_addr_list = alist;
		def.h_addr = (char *)&defaddr;
		def.h_length = sizeof(struct in_addr);
		def.h_addrtype = AF_INET;
		def.h_aliases = 0;
		hp = &def;
	}
	sp = getservbyname("finger", "tcp");
	if (!sp) {
		fprintf(stderr, "finger: tcp/finger: unknown service\n");
		return;
	}
	memset(&sn, 0, sizeof(sn));
	sn.sin_family = hp->h_addrtype;
	if (hp->h_length > (int)sizeof(sn.sin_addr)) {
	    hp->h_length = sizeof(sn.sin_addr);
	}
	memcpy(&sn.sin_addr, hp->h_addr, hp->h_length);
	sn.sin_port = sp->s_port;

	if ((s = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
		perror("finger: socket");
		return;
	}

	/* have network connection; identify the host connected with */
	printf("[%s]\n", hp->h_name);
	if (connect(s, (struct sockaddr *)&sn, sizeof(sn)) < 0) {
		perror("finger: connect");
		close(s);
		return;
	}

	/* -l flag for remote fingerd  */
	if (lflag) write(s, "/W ", 3);

	/* send the name followed by <CR><LF> */
	write(s, name, strlen(name));
	write(s, "\r\n", 2);

	/*
	 * Read from the remote system; once we're connected, we assume some
	 * data.  If none arrives, we hang until the user interrupts.
	 *
	 * If we see a <CR> or a <CR> with the high bit set, treat it as
	 * a newline; if followed by a newline character, only output one
	 * newline.
	 *
	 * Otherwise, all high bits are stripped; if it isn't printable and
	 * it isn't a space, we can simply set the 6th bit.  Every ASCII
	 * character with bit 6 set is printable.
	 */ 
	fp = fdopen(s, "r");
	if (!fp) {
		perror("finger: fdopen");
		close(s);
		return;
	}

	while ((c = getc(fp)) != EOF) {
		/*
		 * Why doesn't this use vputc() from lprint.c?
		 */
		c &= 0x7f;
		if (c == '\r') {
			c = '\n';
			lastc = '\r';
		} 
		else {
			if (!isprint(c) && !isspace(c)) c |= 0x40;
				
			if (lastc != '\r' || c != '\n') lastc = c;
			else {
				lastc = '\n';
				continue;
			}
		}
		putchar(c);
	}
	if (lastc != '\n') putchar('\n');
	fclose(fp);
}
