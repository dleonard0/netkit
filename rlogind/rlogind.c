/*-
 * Copyright (c) 1983, 1988, 1989 The Regents of the University of California.
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
 * Modifications for Linux-PAM: Al Longyear <longyear@netcom.com>
 *   General code clean up: Andrew Morgan <morgan@physics.ucla.edu>
 *   Re-built with #ifdef USE_PAM: Michael K. Johnson <johnsonm@redhat.com>,
 *   Red Hat Software
 *
 *   The Linux-PAM mailing list (25JUN96) <pam-list@redhat.com>
 */

char copyright[] =
  "@(#) Copyright (c) 1983, 1988, 1989 "
  "The Regents of the University of California.\n"
  "All rights reserved.\n";

/* 
 * From: @(#)rlogind.c	5.53 (Berkeley) 4/20/91
 */
char rcsid[] = 
  "$Id: rlogind.c,v 1.20 1996/12/29 17:26:11 dholland Exp $";

/*
 * remote login server:
 *	\0
 *	remuser\0
 *	locuser\0
 *	terminal_type/speed\0
 *	data
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <signal.h>
#include <termios.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <pwd.h>
#include <grp.h>
#include <syslog.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "pathnames.h"
#include "logwtmp.h"

#ifdef USE_PAM
#include <sys/types.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#endif

pid_t forkpty(int *, char *, struct termios *, struct winsize *);
int logout(const char *);

#ifdef GNU_LIBC
#define _check_rhosts_file  __check_rhosts_file
#endif

#ifndef TIOCPKT_WINDOW
#define TIOCPKT_WINDOW 0x80
#endif

#define	ENVSIZE	(sizeof("TERM=")-1)	/* skip null for concatenation */
#define	NMAX 30

static char *env[2];
static char lusername[NMAX+1], rusername[NMAX+1];
static char term[64] = "TERM=";
static int keepalive = 1;
static int check_all = 0;
static struct passwd *pwd;
static int allow_root_rhosts = 0;
static int deny_all_rhosts_hequiv = 0;

#ifdef USE_PAM
static pam_handle_t *pamh;
static int retcode;
#endif /* USE_PAM */

static void doit(int f, struct sockaddr_in *fromp);
static int control(int pty, char *cp, int n);
static void protocol(int f, int p);
static void cleanup(int);
static void fatal(int f, const char *msg, int syserr);
static int do_rlogin(const char *host);
static void getstr(char *buf, int cnt, const char *errmsg);
static void setup_term(int fd);
static void usage(void);
static int local_domain(const char *h);
static const char *topdomain(const char *h);

extern int _check_rhosts_file;

int
main(int argc, char **argv)
{
	int ch;
	int on = 1;
	size_t fromlen;
	struct sockaddr_in from;
	_check_rhosts_file = 1;     /* default */

	openlog("rlogind", LOG_PID | LOG_CONS, LOG_AUTH);

	opterr = 0;
	while ((ch = getopt(argc, argv, "ahLln")) != EOF) {
		switch (ch) {
		case 'a':
			check_all = 1;
			break;
		case 'h':
			allow_root_rhosts = 1;
			break;
		case 'L':
			deny_all_rhosts_hequiv = 1;
			break;
		case 'l':
			_check_rhosts_file = 0;
			break;
		case 'n':
			keepalive = 0;
			break;
		case '?':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

#ifdef USE_PAM
        if (_check_rhosts_file==0 || deny_all_hosts_equiv || allow_root_rhosts)
            syslog(LOG_ERR, "-l, -L, and -h functionality has been moved to "
                            "pam_rhosts_auth in /etc/pam.conf");
#endif

	from.sin_family = AF_INET;
	fromlen = sizeof (from);
	if (getpeername(0, (struct sockaddr *)&from, &fromlen) < 0) {
		syslog(LOG_ERR,"Can't get peer name of remote host: %m");
		fatal(STDERR_FILENO, "Can't get peer name of remote host", 1);
	}
	if (keepalive &&
	    setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (on)) < 0)
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
#ifdef IP_TOS
	on = IPTOS_LOWDELAY;
	if (setsockopt(0, IPPROTO_IP, IP_TOS, (char *)&on, sizeof(int)) < 0)
		syslog(LOG_WARNING, "setsockopt (IP_TOS): %m");
#endif
	doit(0, &from);
	/* NOTREACHED */
	return 0;
}

int	netf;
char	line[MAXPATHLEN];
int	confirmed;

struct winsize win = { 0, 0, 0, 0 };

static void
doit(int f, struct sockaddr_in *fromp)
{
	int master, pid, on = 1;
	int authenticated = 0, hostok = 0;
	struct hostent *hop;
	char *hname;
	char remotehost[2 * MAXHOSTNAMELEN + 1];
	char c;

	alarm(60);
	read(f, &c, 1);

	if (c != 0)
		exit(1);

	alarm(0);
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	hop = gethostbyaddr((char *)&fromp->sin_addr, sizeof(struct in_addr),
			    fromp->sin_family);
	if (hop == 0) {
		/*
		 * Only the name is used below.
		 */
		hname = strdup(inet_ntoa(fromp->sin_addr));
		hostok++;
	} 
	else if (check_all || local_domain(hop->h_name)) {
		/*
		 * If name returned by gethostbyaddr is in our domain,
		 * attempt to verify that we haven't been fooled by someone
		 * in a remote net; look up the name and check that this
		 * address corresponds to the name.
		 */
		strncpy(remotehost, hop->h_name, sizeof(remotehost) - 1);
		remotehost[sizeof(remotehost) - 1] = 0;
		hop = gethostbyname(remotehost);
		if (hop)
		    for (; hop->h_addr_list[0]; hop->h_addr_list++)
			if (!memcmp(hop->h_addr_list[0], &fromp->sin_addr,
				    sizeof(fromp->sin_addr))) {
				hostok++;
				break;
			}
	} else
		hostok++;
	hname = strdup(hop->h_name);
	hop = NULL;  /* we shouldn't use this after this point */

	{
	    if (fromp->sin_family != AF_INET ||
	        fromp->sin_port >= IPPORT_RESERVED ||
	        fromp->sin_port < IPPORT_RESERVED/2) {
		    syslog(LOG_NOTICE, "Connection from %s on illegal port",
			    inet_ntoa(fromp->sin_addr));
		    fatal(f, "Permission denied", 0);
	    }
#ifdef IP_OPTIONS
	    {
	    u_char optbuf[BUFSIZ/3], *cp;
	    char lbuf[BUFSIZ], *lp;
	    size_t optsize = sizeof(optbuf);
	    int ipproto;
	    struct protoent *ip;

	    if ((ip = getprotobyname("ip")) != NULL)
		    ipproto = ip->p_proto;
	    else
		    ipproto = IPPROTO_IP;
	    if (getsockopt(0, ipproto, IP_OPTIONS, (char *)optbuf,
		&optsize) == 0 && optsize != 0) {
		    lp = lbuf;
		    for (cp = optbuf; optsize > 0; cp++, optsize--, lp += 3)
			    sprintf(lp, " %2.2x", *cp);
		    syslog(LOG_NOTICE,
			"Connection received using IP options (ignored):%s",
			lbuf);
		    if (setsockopt(0, ipproto, IP_OPTIONS,
			(char *)NULL, optsize) != 0) {
			    syslog(LOG_ERR, "setsockopt IP_OPTIONS NULL: %m");
			    exit(1);
		    }
	        }
	    }
#endif
	    if (do_rlogin(hname) == 0 && hostok)
		    authenticated++;
	}
	if (confirmed == 0) {
		write(f, "", 1);
		confirmed = 1;		/* we sent the null! */
	}
	   if (!authenticated && !hostok)
		write(f, "rlogind: Host address mismatch.\r\n",
		    sizeof("rlogind: Host address mismatch.\r\n") - 1);

	netf = f;

	pid = forkpty(&master, line, NULL, &win);
	if (pid < 0) {
		if (errno == ENOENT)
			fatal(f, "Out of ptys", 0);
		else
			fatal(f, "Forkpty", 1);
	}
	if (pid == 0) {
		if (f > 2)	/* f should always be 0, but... */ 
			(void) close(f);
		setup_term(0);
		if (authenticated) {
#ifdef USE_PAM
                       pam_end(pamh, PAM_SUCCESS);
#endif
		       execl(_PATH_LOGIN, "login", "-p",
			     "-h", hname, "-f", lusername, 0);
                       /* should not return... */
		} 
		else {
			if (*lusername=='-') {
				syslog(LOG_AUTH|LOG_INFO, "rlogin with an option as a name!");
				exit(1);
			}
#ifdef USE_PAM
			pam_end(pamh, PAM_SUCCESS);
#endif
			execl(_PATH_LOGIN, "login", "-p",
			      "-h", hname, lusername, 0);
			/* should not return... */
		}
		fatal(STDERR_FILENO, _PATH_LOGIN, 1);
		/*NOTREACHED*/
	}
	ioctl(f, FIONBIO, &on);
	ioctl(master, FIONBIO, &on);
	ioctl(master, TIOCPKT, &on);
	signal(SIGCHLD, cleanup);
	protocol(f, master);
	signal(SIGCHLD, SIG_IGN);
	cleanup(0);
}

char	magic[2] = { 0377, 0377 };
char	oobdata[] = {TIOCPKT_WINDOW};

/*
 * Handle a "control" request (signaled by magic being present)
 * in the data stream.  For now, we are only willing to handle
 * window size changes.
 */
static int
control(int pty, char *cp, int n)
{
	struct winsize w;

	if (n < 4+(int)sizeof(w) || cp[2] != 's' || cp[3] != 's') {
		return 0;
	}
	oobdata[0] &= ~TIOCPKT_WINDOW;	/* we know he heard */
	memcpy(&w, cp+4, sizeof(w));
	w.ws_row = ntohs(w.ws_row);
	w.ws_col = ntohs(w.ws_col);
	w.ws_xpixel = ntohs(w.ws_xpixel);
	w.ws_ypixel = ntohs(w.ws_ypixel);
	ioctl(pty, TIOCSWINSZ, &w);
	return 4+sizeof(w);
}

/*
 * rlogin "protocol" machine.
 */
static void
protocol(int f, int p)
{
	char pibuf[1024+1], fibuf[1024], *pbp = NULL, *fbp = NULL;
	register pcc = 0, fcc = 0;
	int cc, nfd, m;
	char cntl;

	/*
	 * Must ignore SIGTTOU, otherwise we'll stop
	 * when we try and set slave pty's window shape
	 * (our controlling tty is the master pty).
	 */
	(void) signal(SIGTTOU, SIG_IGN);
	send(f, oobdata, 1, MSG_OOB);	/* indicate new rlogin */
	if (f > p)
		nfd = f + 1;
	else
		nfd = p + 1;
	if (nfd > FD_SETSIZE) {
		syslog(LOG_ERR, "select mask too small, increase FD_SETSIZE");
		fatal(f, "internal error (select mask too small)", 0);
	}
	for (;;) {
		fd_set ibits, obits, ebits, *omask;

		FD_ZERO(&ebits);
		FD_ZERO(&ibits);
		FD_ZERO(&obits);
		omask = (fd_set *)NULL;
		if (fcc) {
			FD_SET(p, &obits);
			omask = &obits;
		} else
			FD_SET(f, &ibits);
		if (pcc >= 0)
			if (pcc) {
				FD_SET(f, &obits);
				omask = &obits;
			} else
				FD_SET(p, &ibits);
		FD_SET(p, &ebits);
		if ((m = select(nfd, &ibits, omask, &ebits, 0)) < 0) {
			if (errno == EINTR)
				continue;
			fatal(f, "select", 1);
		}
		if (m == 0) {
			/* shouldn't happen... */
			sleep(5);
			continue;
		}
#define	pkcontrol(c)	((c)&(TIOCPKT_FLUSHWRITE|TIOCPKT_NOSTOP|TIOCPKT_DOSTOP))
		if (FD_ISSET(p, &ebits)) {
			cc = read(p, &cntl, 1);
			if (cc == 1 && pkcontrol(cntl)) {
				cntl |= oobdata[0];
				send(f, &cntl, 1, MSG_OOB);
				if (cntl & TIOCPKT_FLUSHWRITE) {
					pcc = 0;
					FD_CLR(p, &ibits);
				}
			}
		}
		if (FD_ISSET(f, &ibits)) {
				fcc = read(f, fibuf, sizeof(fibuf));
			if (fcc < 0 && errno == EWOULDBLOCK)
				fcc = 0;
			else {
				register char *cp;
				int left, nn;

				if (fcc <= 0)
					break;
				fbp = fibuf;

			top:
				for (cp = fibuf; cp < fibuf+fcc-1; cp++)
					if (cp[0] == magic[0] &&
					    cp[1] == magic[1]) {
						left = fcc - (cp-fibuf);
						nn = control(p, cp, left);
						if (nn) {
							left -= nn;
							if (left > 0)
								bcopy(cp+nn, cp, left);
							fcc -= nn;
							goto top; /* n^2 */
						}
					}
				FD_SET(p, &obits);		/* try write */
			}
		}

		if (FD_ISSET(p, &obits) && fcc > 0) {
			cc = write(p, fbp, fcc);
			if (cc > 0) {
				fcc -= cc;
				fbp += cc;
			}
		}

		if (FD_ISSET(p, &ibits)) {
			pcc = read(p, pibuf, sizeof (pibuf));
			pbp = pibuf;
			if (pcc < 0 && errno == EWOULDBLOCK)
				pcc = 0;
			else if (pcc <= 0)
				break;
			else if (pibuf[0] == 0) {
				pbp++, pcc--;
					FD_SET(f, &obits);	/* try write */
			} else {
				if (pkcontrol(pibuf[0])) {
					pibuf[0] |= oobdata[0];
					send(f, &pibuf[0], 1, MSG_OOB);
				}
				pcc = 0;
			}
		}
		if ((FD_ISSET(f, &obits)) && pcc > 0) {
				cc = write(f, pbp, pcc);
			if (cc < 0 && errno == EWOULDBLOCK) {
				/*
				 * This happens when we try write after read
				 * from p, but some old kernels balk at large
				 * writes even when select returns true.
				 */
				if (!FD_ISSET(p, &ibits))
					sleep(5);
				continue;
			}
			if (cc > 0) {
				pcc -= cc;
				pbp += cc;
			}
		}
	}
}

static void
cleanup(int ignore)
{
	char *p;
	(void)ignore;

	p = line + sizeof(_PATH_DEV) - 1;
	if (logout(p))
		logwtmp(p, "", "");
#ifdef USE_PAM
       pam_end (pamh, PAM_SUCCESS);
#endif

	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);
	*p = 'p';
	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);
	shutdown(netf, 2);
	exit(1);
}

static void
fatal(int f, const char *msg, int syserr)
{
	int len;
	char buf[BUFSIZ], *bp = buf;

	/*
	 * Prepend binary one to message if we haven't sent
	 * the magic null as confirmation.
	 */
	if (!confirmed)
		*bp++ = '\01';		/* error indicator */

	switch (syserr) {
	  case 0:
		len = snprintf(bp, sizeof(buf)-1, "rlogind: %s.\r\n", msg);
		break;
	  default:
  	  case 1:
		len = snprintf(bp, sizeof(buf)-1, "rlogind: %s: %s.\r\n",
			       msg, strerror(errno));
		break;
#ifdef USE_PAM
	  case 2:
		len = snprintf(bp, sizeof(buf)-1, "rlogind: %s: %s.\r\n",
			       msg, pam_strerror(errno));
		break;
#endif
	}

	write(f, buf, bp + len - buf);

#ifdef USE_PAM
	pam_end (pamh, PAM_SUCCESS);
#endif

	exit(1);
}

static int
do_rlogin(const char *host)
{
#ifdef USE_PAM
	char c;
	static struct pam_conv conv = {
	  misc_conv,
	  NULL
	};
        int retval;
#endif /* USE_PAM */

	getstr(rusername, sizeof(rusername), "remuser too long");
	getstr(lusername, sizeof(lusername), "locuser too long");
	getstr(term+ENVSIZE, sizeof(term)-ENVSIZE, "Terminal type too long");

	pwd = getpwnam(lusername);
	if (pwd == NULL)
		return(-1);

#ifdef USE_PAM
       retcode = pam_start("rlogin", lusername, &conv, &pamh);
       if (retcode != PAM_SUCCESS) {
               syslog (LOG_ERR, "pam_start: %s\n", pam_strerror(retcode));
               fatal(STDERR_FILENO, "initialization failed", 0);
       }

       (void) pam_set_item (pamh, PAM_USER,  lusername);
       (void) pam_set_item (pamh, PAM_RUSER, rusername);
       (void) pam_set_item (pamh, PAM_RHOST, host);
       (void) pam_set_item (pamh, PAM_TTY, "tty");

       c = 0;
       write (0, &c, 1);
       confirmed = 1;

       do {
               retval = pam_authenticate(pamh, 0);
               if (retval == PAM_SUCCESS)
                       retval = pam_acct_mgmt(pamh, 0);
               if (retval == PAM_SUCCESS)
                       break;
               if (retval == PAM_AUTHTOKEN_REQD) {
                       retval = pam_chauthtok (pamh,PAM_CHANGE_EXPIRED_AUTHTOK);
	               if(retval == PAM_SUCCESS)
			 /* Try authentication again if passwd change
			    succeeded.  Don't try again if it didn't;
			    sysadmin might not want passwords changed
			    over the next, and might have set password
			    to pam_deny.so to disable it... */
			 continue;
	       }
       } while (0); /* We have the while(0) here because it is either using
		       that and the breaks, or goto's */
	/* eww. -dah */


	if (retval == PAM_SUCCESS) {
               if (setgid(pwd->pw_gid) != 0) {
                       fprintf(stderr, "cannot assume gid\n");
                       return (0);
               }

               if (initgroups(lusername, pwd->pw_gid) != 0) {
                       fprintf(stderr, "cannot initgroups\n");
                       return (0);
               }

               retval = pam_setcred(pamh, PAM_CRED_ESTABLISH);
	}

	if (retval != PAM_SUCCESS) {
		syslog(LOG_ERR,"PAM authentication failed for in.rlogind");
		fatal(STDERR_FILENO, "login failed", 0);
		/* no return */
	}
	return 0;

#else /* !USE_PAM */

	if (deny_all_rhosts_hequiv) {
		return -1;
	}
	if (!allow_root_rhosts && pwd->pw_uid == 0) {
		return -1;
	}
	return(ruserok(host, 0, rusername, lusername));
#endif /* PAM */
}

static void
getstr(char *buf, int cnt, const char *errmsg)
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		if (--cnt < 0)
			fatal(STDOUT_FILENO, errmsg, 0);
		*buf++ = c;
	} while (c != 0);
}

extern	char **environ;

static void
setup_term(int fd)
{
	register char *cp = strchr(term+ENVSIZE, '/');
	char *speed;
	struct termios tt;

	tcgetattr(fd, &tt);
	if (cp) {
		*cp++ = '\0';
		speed = cp;
		cp = index(speed, '/');
		if (cp)
			*cp++ = '\0';
		cfsetispeed(&tt, atoi(speed));
		cfsetospeed(&tt, atoi(speed));
	}
#if 0  /* notyet */
	tt.c_iflag = TTYDEF_IFLAG;
	tt.c_oflag = TTYDEF_OFLAG;
	tt.c_lflag = TTYDEF_LFLAG;
#endif
	tcsetattr(fd, TCSAFLUSH, &tt);

	env[0] = term;
	env[1] = 0;
	environ = env;
}


static void
usage(void)
{
	syslog(LOG_ERR, "usage: rlogind [-aln]");
}

/*
 * Check whether host h is in our local domain,
 * defined as sharing the last two components of the domain part,
 * or the entire domain part if the local domain has only one component.
 * If either name is unqualified (contains no '.'),
 * assume that the host is local, as it will be
 * interpreted as such.
 */
static int
local_domain(const char *h)
{
	char localhost[MAXHOSTNAMELEN];
	const char *p1, *p2;

	localhost[0] = 0;
	(void) gethostname(localhost, sizeof(localhost));
	p1 = topdomain(localhost);
	p2 = topdomain(h);
	if (p1 == NULL || p2 == NULL || !strcasecmp(p1, p2))
		return(1);
	return(0);
}

static const char *
topdomain(const char *h)
{
	const char *p;
	const char *maybe = NULL;
	int dots = 0;

	for (p = h + strlen(h); p >= h; p--) {
		if (*p == '.') {
			if (++dots == 2)
				return (p);
			maybe = p;
		}
	}
	return (maybe);
}
