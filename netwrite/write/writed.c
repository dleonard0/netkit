/*
 * This code is copyrighted. Please see the file COPYING for a full notice.
 */

/*
 * writed - Receive network write connections with inetd.
 *
 * Note:  This is mostly fingerd.c with all occurances of
 * "finger" changed to "write". Well, not so much any more.
 *
 * $Log: writed.c,v $
 * Revision 1.5  1997/06/09 01:32:03  dholland
 * minor glibc fixes
 *
 * Revision 1.4  1997/03/08 17:24:30  dholland
 * Oops, fix warning.
 *
 * Revision 1.3  1997/03/08 12:42:04  dholland
 * Don't depend on gcc extensions
 *
 * Revision 1.2  1996/11/25 18:43:05  dholland
 * clean compile.
 *
 * Revision 1.1  1996/11/23  19:50:40  dholland
 * Initial revision
 *
 * Revision 1.8  1996/04/29  21:43:25  dholland
 * Copyright fixes.
 *
 * Revision 1.7  1996/04/29  20:25:43  dholland
 * Config improvements.
 *
 * Revision 1.6  1996/04/29  19:59:14  dholland
 * Add RCS stuff.
 *
 */

char copyright[] =
  "@(#) Copyright (c) 1983 Regents of the University of California.\n"
  "All rights reserved.\n";
/*
 * From: /afs/rel-eng.athena.mit.edu/project/release/current/
 *        source/bsd-4.3/common/etc/RCS/writed.c,v 
 *   1.3 90/04/05 18:31:44 epeisach Exp
 */
char rcsid[] = "$Id: writed.c,v 1.5 1997/06/09 01:32:03 dholland Exp $";

/*
 * Write server.
 */
/* #include <mit-copyright.h> */

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#ifdef ultrix  /* missing protos */
struct sockaddr;
int getpeername(int fd, struct sockaddr *, int *len);
#endif

/* Should be set by makefile */
#ifndef BINDIR
#define BINDIR "/usr/local/bin"
#endif

#define WS " \t\r\n"
#define AC 6  /* max # of args to write, incl terminating null */

/*
#define DEBUG
*/

static void fatal2(char *t) {
  printf("%s\n", t);
  exit(1);
}

static void fatal(char *s, char *t) {
  if (errno!=0) printf("%s: %s\n", s, strerror(errno));
  fatal2(t);
}

static void secure(const char *fromarg) {
  struct hostent *ho;
  struct sockaddr_in sin;
  size_t x = sizeof(sin);
  char *u, *h, *t;
  u = strdup(fromarg);
  if (!u) fatal("strdup", "Out of memory");
  h = strchr(u, '@');
  if (!h) fatal2("Usage: write user [tty]");
  *h++=0;
  t = strchr(h, '@');
  if (t) {
    *t++=0;
    if (strchr(t, '@')) fatal2("Usage: write user [tty]");
  }

  ho = gethostbyname(h);
  if (!ho) fatal("gethostbyname", "Where are you?");
  if (getpeername(0, (struct sockaddr *) &sin, &x) < 0) {
    fatal("getpeername", "Where are you?");
  }
  free(u);

  for (x=0; ho->h_addr_list[x]; x++) {
#ifdef DEBUG
    unsigned char *p = ho->h_addr_list[x];
    unsigned char *q = (char *) &sin.sin_addr;
    printf("Testing: %u.%u.%u.%u vs %u.%u.%u.%u [%d]\n", 
	   p[0], p[1], p[2], p[3], q[0], q[1], q[2], q[3], ho->h_length);
    fflush(stdout);
#endif
    if (ho->h_length > (int)sizeof(sin.sin_addr)) {
	ho->h_length = sizeof(sin.sin_addr);
    }
    if (!memcmp(ho->h_addr_list[x], (char *) &sin.sin_addr, ho->h_length))
      return;
  }
  fatal2("Host name lookup error");
}


int main(void /*int argc, char *argv[]*/) {
  char line[BUFSIZ], *av[AC];
  int i=2;
  av[0] = "write";
  av[1] = "-f";
  *line=0;
#ifdef DEBUG
  printf("writed ready\n");
  fflush(stdout);
#endif
  fgets(line, BUFSIZ, stdin);
  for (av[i]=strtok(line, WS); av[i++] && i<AC; av[i]=strtok(NULL, WS));
  av[AC-1] = NULL;
  dup2(0, 1);
  dup2(0, 2);
  secure(av[2]);
#ifdef DEBUG
  printf("Ok, all clear\n");
  fflush(stdout);
#endif
  execv(BINDIR "/write", av);
  execv("/usr/local/bin/write", av);
  execv("/usr/bin/write", av);
  execv("/bin/write", av);
  fatal("execv", "Can't find binary for write");
  return 0;
}
