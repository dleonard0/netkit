/*
 * This code is copyrighted. Please see the file COPYING for a full notice.
 */

/*
 * write.c - Write to another user's tty
 *
 * $Log: write.c,v $
 * Revision 1.8  1997/04/06 01:40:49  dholland
 * better utmp handling
 *
 * Revision 1.7  1997/03/08 12:41:48  dholland
 * Fix for irix
 *
 * Revision 1.6  1997/01/19 06:16:00  dholland
 * in hunt_utmp, savebuf should be the same size as ttybuf
 *
 * Revision 1.5  1997/01/06  05:40:59  dholland
 * use pathnames.h
 *
 * Revision 1.4  1996/12/29  17:36:47  dholland
 * fix potential buffer overrun
 *
 * Revision 1.3  1996/11/25  18:52:59  dholland
 * better handling of CHECK_UT_TYPE
 *
 * Revision 1.2  1996/11/25  18:43:25  dholland
 * clean compile; fix h_length issues; use inet_aton instead of inet_addr
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

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <utmp.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pwd.h>
#include <unistd.h>
#include "pathnames.h"

#define	NMAX	sizeof(((struct utmp *)NULL)->ut_name)  /* hack */
#define	LMAX	sizeof(((struct utmp *)NULL)->ut_line)

/*
 * From: 
 * /afs/athena.mit.edu/astaff/project/olcdev/src/server/writed/RCS/write.c,v 
 *     1.1 91/05/15 12:36:40 lwvanels Exp
 *
 * @(#)write.c	4.13 3/13/86
 */

char rcsid[] = "$Id: write.c,v 1.8 1997/04/06 01:40:49 dholland Exp $";

/*********************** conditionals for ports ************************/

#ifdef _AIX
#define SYSVISH_SETPGRP
#endif

#ifdef __linux__
#define SYSVISH_SETPGRP
#endif

#ifdef __hpux__
#define SYSVISH_SETPGRP
#define CHECK_UT_TYPE
#endif

#ifdef ultrix
  char *rindex(const char *, int);
  int socket(int af, int type, int protocol);
  void bcopy(char *b1, const char *b2, int length);
  int connect(int s, struct sockaddr *, int len);
  int setpgrp (int pid, int pgrp);
  int gethostname(char *, int);
#endif

/*
 * In sysV, and sysVish or POSIX environments like Linux, setpgrp() takes
 * no arguments, and what we want is actually called setpgid. 
 */
#ifdef SYSVISH_SETPGRP
#define setpgrp setpgid
#endif

/*
 * Just in case.
 */
#if defined(USER_PROCESS) && !defined(CHECK_UT_TYPE)
#define CHECK_UT_TYPE
#endif

/*
 * Some types.
 */

typedef void (*sigfunc)(int);

typedef struct {
    const char *user;
    const char *host;
    const char *tty;
    const char *asktty;
    int fromnet;
    int authentic;
} person;


/**************************** lookup operations **************************/

/*
 * Get local "me" tty
 */
static char *getmytty(void) {
  struct stat stbuf;
  char *mytty = ttyname(2);
  if (!mytty) {
    fprintf(stderr, "write: Can't find your tty\n");
    exit(1);
  }
  if (stat(mytty, &stbuf) < 0) {
    perror("write: Can't stat your tty");
    exit(1);
  }
  if ((stbuf.st_mode&020) == 0) {
    fprintf(stderr, "write: You have write permission turned off.\n");
    if (getuid()!=0) exit(1);
  }
  mytty += 5; /* skip /dev/ */  /* strchr(mytty, '/') + 1; */
  return mytty;
}

/*
 * Search utmp file
 */
static int hunt_utmp(person *himm) {
  static char savebuf[LMAX+5];
  int gothim = 0, logcnt=0;
  struct stat stbuf;
  struct utmp *uptr;
  char tempbuf[NMAX+1];
  char ttybuf[LMAX+5];
  setutent();
  while ((uptr = getutent())!=NULL) {
#ifdef CHECK_UT_TYPE
    if (uptr->ut_type != USER_PROCESS) continue;
#endif
    if (uptr->ut_name[0] == '\0') continue;
    memcpy(tempbuf, uptr->ut_name, NMAX);  /* ut_name doesn't necessarily */
    tempbuf[NMAX]=0;                      /* come null-terminated... */
    if (strchr(tempbuf, ' ')) *strchr(tempbuf, ' ')=0;
    if (strcmp(himm->user, tempbuf)) continue;
    if (himm->asktty && strcmp(himm->asktty, uptr->ut_line)) continue;
    logcnt++;
    if (gothim) continue;
    strcpy(ttybuf, "/dev/");
    strcat(ttybuf, uptr->ut_line);
    if (!stat(ttybuf, &stbuf) && (stbuf.st_mode&020)) {
      strcpy(savebuf, ttybuf);
      gothim=1;
    }
  }
  endutent();
  himm->tty = savebuf;
  return logcnt;
}

/*
 * Go grub for the target [local] user in /etc/utmp.
 */
static void doutmp(person *him) {
  int logcnt = hunt_utmp(him);
  if (logcnt==0) {
    fprintf(stderr, "write: %s not logged in%s\n", him->user,
	    him->asktty ? " on that tty" : "");
    exit(1);
  }
  if (!him->asktty && logcnt > 1) {
    fprintf(stderr, "write: %s logged in more than once ... writing to %s\n",
	    him->user, him->tty+5);
  }
/*#ifdef _AIX*/
  /* This appears to flush the stderr buffer on the PS/2 so that
     both sides of the connection don't receive the message. 
     Weird. - Ezra */
  /* HP-UX has the same problem, but flushing stderr doesn't solve it.
     I'm leaving the flush in, because it's not a bad idea anyway. See
     below for the real fix. - dholland */
  fflush(stderr);
/*#endif*/
}

/********************* open socket for net write **************************/

/*
 * Open write socket to specified host.
 */
static FILE *opensocket(struct hostent *hp) {
  FILE *f;
  int fds;
  struct sockaddr_in sin;
  struct servent *sp = getservbyname("write", "tcp");
  if (sp == 0) {
    fprintf(stderr, "tcp/write: unknown service\n");
    exit(1);
  }
  sin.sin_family = hp->h_addrtype;
  if (hp->h_length > (int)sizeof(sin.sin_addr)) {
    hp->h_length = sizeof(sin.sin_addr);
  }
  memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
  sin.sin_port = sp->s_port;
  fds = socket(hp->h_addrtype, SOCK_STREAM, 0);
  if (fds < 0) {
    perror("socket");
    exit(1);
  }
  if (connect(fds, (struct sockaddr *)&sin, sizeof (sin)) < 0) {
    perror("connect");
    close(fds);
    exit(1);
  }
  f = fdopen(fds, "r+");
  if (!f) {
    perror("fdopen");
    close(fds);
    exit(1);
  }
  return f;
}

/*
 * Translate machine name into hostent.
 */
static struct hostent *find_remote(const char *hishost) {
  struct hostent *hp;
  hp = gethostbyname(hishost);
  if (hp == NULL) {
    static struct hostent def;
    static struct in_addr defaddr;
    static char namebuf[256];
    
    if (!inet_aton(hishost, &defaddr)) {
      printf("unknown host: %s\n", hishost);
      exit(1);
    }
    strncpy(namebuf, hishost, sizeof(namebuf));
    namebuf[sizeof(namebuf)-1] = 0;
    def.h_name = namebuf;
    def.h_addr = (char *)&defaddr;
    def.h_length = sizeof (struct in_addr);
    def.h_addrtype = AF_INET;
    def.h_aliases = 0;
    hp = &def;
  }
  return hp;
}

/*
 * Open a remote machine for writing.
 */
static FILE *open_to_remote(const person *me, const person *him) {
  char buf[128];
  struct hostent *hp = find_remote(him->host);
  FILE *tf = opensocket(hp);
  fprintf(tf, "%s@%s@%s %s%s%s\r\n", me->user, me->host, me->tty, 
	  him->user, him->asktty ? " " : "", him->asktty ? him->asktty : "");
  fflush(tf);
  while (1) {
    if (fgets(buf, sizeof(buf), tf) == NULL) exit(1);
    if (buf[0] == '\n') break;
    write(1, buf, strlen(buf));
  }
  fflush(tf);   /* Apparently this is needed in Irix */
  return tf;
}

/*********************** open a tty for local write ******************/

/*
 * Signal handler for timeout
 */
static void timout(int ignore) {
  (void)ignore;
  fprintf(stderr, "write: open tty: timed out\n");
  exit(1);
}

/*
 * Open a local tty for writing.
 */
static FILE *open_to_local(const person *me, const person *him) {
  FILE *tf;
  int fd;
  struct stat stbuf;
  time_t clock = time(NULL);
  struct tm *localclock = localtime( &clock );

  signal(SIGALRM, timout);
  alarm(5);
/*
#ifndef ultrix
  if (setpgrp(0,0)) perror("setpgrp 0");
#endif
*/
  if (stat(him->tty, &stbuf) < 0 || (stbuf.st_mode&020) == 0
      || (fd = open(him->tty, O_WRONLY|O_NOCTTY)) < 0) {
    fprintf(stderr, "write: Permission denied\n");
    exit(1);
  }
  tf = fdopen(fd, "w");
  if (!tf) { 
    fprintf(stderr, "write: fdopen failed\n");
    exit(1);
  }
/*
  if (setpgrp(0,getpid())) perror("setpgrp !0");
*/
  alarm(0);
  
  if (me->fromnet) {
    printf("\n");
    fflush(stdout);
  }
  fprintf(tf, "\r\nMessage from %s@%s%s%s %sat %d:%02d ...\r\n\007\007\007",
	  me->user, me->host, me->tty ? " on " : "", me->tty ? me->tty : "", 
	  me->authentic ? "" : "[UNAUTHENTICATED] ",
	  localclock->tm_hour, localclock->tm_min);
  fflush(tf);
  return tf;
}

/***************************** main loop ***********************************/

static int mainloop_done=0;

/*
 * Catch a bunch of signals and cause mainloop to exit.
 * closing stdin is a hack that causes the read call the main loop is 
 * blocked in to return immediately.
 */
static void catch(int ignore) {
  (void)ignore;
  mainloop_done=1;
  close(0);
}

/*
 * Assumes all 8-bit (>=128) characters are valid and printable (maybe dumb...)
 */
static int printc(int ch, FILE *tf) {
  if (ch=='\n') { fputs("\r\n", tf); fflush(tf); }
  else if (strchr("\r\t ", ch) || (ch>32 && ch<127) || ch<0 || ch>127) fputc(ch, tf);
  else if (ch==127) fputs("^?", tf);
  else { fputc('^', tf); fputc(ch|64, tf); }
  if (ferror(tf) || feof(tf)) return -1;
  return 0;
}

/*
 * Main loop. 
 */
static void writeloop (FILE *tf, int nethim, const char *hisname) {
  char buf[BUFSIZ];
  int j,len;
  signal(SIGHUP, catch);
  signal(SIGINT, catch);
  signal(SIGQUIT, catch);
  while(!mainloop_done) {
    len = read(0, buf, sizeof(buf));
    if (len<=0) break;
    for (j=0; j<len && !mainloop_done; j++) 
      if (printc(buf[j], tf)) {
	printf("\n\007Write failed (%s logged out?)\n", hisname);
	exit(1);
      }
  }
  if (!nethim) {
    fprintf(tf, "EOF\r\n");
    fflush(tf);
  }
}

/**************************** setup *************************************/

/*
 * Fill in the person structure for the target.
 */
static void makehim(person *him, char *himarg, const char *histtya) {
  char *x;
  him->tty = NULL;
  him->asktty = histtya;
  him->user = himarg;
  him->authentic = 1;  /* not really relevant anyway */
  x = strchr(himarg, '@');
  if (x) {
    *x++=0;
    if (strchr(x, '@')) {
      fprintf(stderr, "Usage: write user [tty]\n");
      exit(1);
    }
    him->host = x;
    him->fromnet = 1;
  }
  else {
    him->host = NULL;
    him->fromnet = 0;
    doutmp(him);
  }
}

/*
 * Convert "net arg" of the form user@host@tty and fill in person structure.
 * We may get just user@host though.
 */
static int makeme_net(person *me, char *netarg, int authentic) {
  char *x;

  /*
   * It appears that on some systems (including AIX and HP-UX), inetd
   * doesn't bother to open stderr. This leads to the file opened to
   * the user also being stderr, with two different FILE * open on it,
   * which causes just absolutely wonderful things to happen.
   * This should fix the problem and be harmless on nonbroken systems.
   * Gotta love System V.
   * dholland 4/28/96
   *
   * Actually, this is more puzzling than I thought, since writed does
   * dup2(0,1); dup2(0,2); ... and yet this fix caused the bug to go away.
   * Still very strange.
   * dholland 4/29/96
   */
  dup2(1,2);

  me->user = netarg;
  me->asktty = NULL;
  me->tty = NULL;
  me->fromnet = 1;
  me->authentic = authentic;
  x = strchr(netarg, '@');
  if (!x) return -1;                /* no @'s */
  *x++ = 0;
  me->host = x;
  x = strchr(x, '@');
  if (x) {
    *x++=0;
    me->tty = x;
    if (strchr(x, '@')) return -1;  /* too many @'s */
  }
  return 0;
}

/*
 * Construct person structure based on where we are.
 * If we can't getlogin(), people can't write back, so deny access.
 */
static void makeme_local(person *me) {
  static char hostname[128];
  struct hostent *h;
  gethostname(hostname, sizeof(hostname));
  h = gethostbyname(hostname);
  if (h) {   /* if we fail, pretend we didn't try */
    strncpy(hostname, h->h_name, sizeof(hostname));
    hostname[sizeof(hostname)-1] = 0; /* ensure null termination */
  }

  me->user = getlogin();
  me->host = hostname;
  me->tty = getmytty();
  me->asktty = NULL;
  me->fromnet = 0;
  me->authentic = 1;
  if (!me->user) {
    if (getuid()) {
      fprintf(stderr, "Where are you?\n");
      exit(1);
    }
    else {
      fprintf(stderr, "Warning - you aren't really logged in...\n");
      me->user = "root";
    }
  }
}



/*
 * Accept arguments of the form
 *   write [-{f|a} from@fromhost@fromtty] user@host [tty]
 * -{f|a} is reserved for root.
 * -a indicates that the username on the remote host is authenticated.
 */

static void usage(void) {
  fprintf(stderr, "Usage: write user [ttyname]\n");
  exit(1);
}

int main(int argc, char *argv[]) {
  person me, him;
  FILE *tf;
  if (argc<2 || argc>5) usage();
  if (!strcmp(argv[1], "-f") || !strcmp(argv[1], "-a")) {
    if (getuid()!=0 || argc<4) usage();
    if (makeme_net(&me, argv[2], !strcmp(argv[1], "-a"))) usage();
    argv += 2;
  }
  else if (argc>3) usage();
  else makeme_local(&me);
  makehim(&him, argv[1], argv[2]);
  if (me.fromnet && him.fromnet) usage();
  if (him.fromnet) tf = open_to_remote(&me, &him);
  else tf = open_to_local(&me, &him); 

  writeloop(tf, him.fromnet, him.user); 
  fclose(tf);
  return 0;
}
