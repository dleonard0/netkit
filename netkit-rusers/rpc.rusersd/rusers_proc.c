#/*-
 *  Copyright (c) 1993 John Brezak
 *  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

char rp_rcsid[] = 
  "$Id: rusers_proc.c,v 1.16 2000/07/23 03:11:56 dholland Exp $";

#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <utmp.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/stat.h>
#ifdef XIDLE
#include <setjmp.h>
#include <X11/Xlib.h>
#include <X11/extensions/xidle.h>
#endif

/*
 * Sigh.
 */
#ifdef __GLIBC__
	#define UT_TIME ut_xtime
	#define RUT_TIME rut_time
#else
	#define UT_TIME ut_time
	#define RUT_TIME ut_time
#endif

/* Glibc strikes again */
#ifdef __GLIBC__
	#include <rpcsvc/rusers.h>
#else
	#include "rusers.h"
#endif 

void rusers_service(struct svc_req *rqstp, SVCXPRT *transp);

#define	IGNOREUSER	"sleeper"

#ifndef _PATH_DEV
#define _PATH_DEV "/dev"
#endif

#ifndef UT_LINESIZE
#define UT_LINESIZE sizeof(((struct utmp *)0)->ut_line)
#endif
#ifndef UT_NAMESIZE
#define UT_NAMESIZE sizeof(((struct utmp *)0)->ut_name)
#endif
#ifndef UT_HOSTSIZE
#define UT_HOSTSIZE sizeof(((struct utmp *)0)->ut_host)
#endif

typedef char ut_line_t[UT_LINESIZE+1];
typedef char ut_name_t[UT_NAMESIZE+1];
typedef char ut_host_t[UT_HOSTSIZE+1];

struct rusers_utmp utmps[MAXUSERS];
struct utmpidle *utmp_idlep[MAXUSERS];
struct utmpidle utmp_idle[MAXUSERS];
ut_line_t line[MAXUSERS];
ut_name_t name[MAXUSERS];
ut_host_t host[MAXUSERS];

extern int from_inetd;

#ifdef XIDLE
Display *dpy;

static sigjmp_buf openAbort;

static void
abortOpen ()
{
    siglongjmp (openAbort, 1);
}

static long
XqueryIdle(char *display)
{
        int first_event, first_error;
        Time IdleTime;
	int result = -1;

        (void) signal (SIGALRM, abortOpen);
        (void) alarm ((unsigned) 10);
        if (!sigsetjmp(openAbort, 1)) {
                if (!(dpy= XOpenDisplay(display))) {
                        syslog(LOG_ERR, "Cannot open display %s", display);
                        goto out;
                }
                if (XidleQueryExtension(dpy, &first_event, &first_error)) {
                        if (!XGetIdleTime(dpy, &IdleTime)) {
                                syslog(LOG_ERR, "%s: Unable to get idle time.", display);
                                goto out;
                        }
                }
                else {
                        syslog(LOG_ERR, "%s: Xidle extension not loaded.", display);
                        goto out;
                }
                XCloseDisplay(dpy);
        }
        else {
                syslog(LOG_ERR, "%s: Server grabbed for over 10 seconds.", display);
                goto out;
        }

        IdleTime /= 1000;
        result = (IdleTime + 30) / 60;

out:
        (void) signal (SIGALRM, SIG_DFL);
        (void) alarm ((unsigned) 0);
	return result;
}
#endif

static u_int
getidle(char *tty, char *display)
{
        struct stat st;
        char devname[PATH_MAX];
        time_t now;
        long idletime;
        
	(void)display;

	/* Don't trust utmp data */
	if (strstr(tty, "../"))
		return 0;

        /*
         * If this is an X terminal or console, then try the
         * XIdle extension
         */
#ifdef XIDLE
        if (display && *display && (idletime = XqueryIdle(display)) >= 0)
                return(idletime);
#endif
        idletime = 0;
#if 0
        if (*tty == 'X') {
                u_long kbd_idle, mouse_idle;
#if	!defined(i386)
                kbd_idle = getidle("kbd", NULL);
#else
#if (__GNUC__ >= 2)
#warning i386 console hack here
#endif
                kbd_idle = getidle("vga", NULL);
#endif
                mouse_idle = getidle("mouse", NULL);
                idletime = (kbd_idle < mouse_idle)?kbd_idle:mouse_idle;
        }
        else {
#endif
	{
                snprintf(devname, sizeof(devname), "%s/%s", _PATH_DEV, tty);
                if (stat(devname, &st) < 0) {
#ifdef DEBUG
                        printf("%s: %s\n", devname, strerror(errno));
#endif
                        return(-1);
                }
                time(&now);
#ifdef DEBUG
                printf("%s: now=%d atime=%d\n", devname, now,
                       st.st_atime);
#endif
		if (now < st.st_atime) idletime = 0;
                else idletime = now - st.st_atime;
                idletime = (idletime + 30) / 60; /* secs->mins */
        }
	/* idletime is unsigned */
        /* if (idletime < 0) idletime = 0; */

        return idletime;
}

static        
char *
rusers_num(void *ign1, struct svc_req *ign2)
{
        static int num_users = 0;
	struct utmp *uptr;
	(void)ign1;
	(void)ign2;

        /* only use entries with both name and line fields */

	setutent();
        while ((uptr = getutent())!=NULL) {
                if (*uptr->ut_name && *uptr->ut_line &&
		    strncmp(uptr->ut_name, IGNOREUSER,
                            sizeof(uptr->ut_name))
#ifdef USER_PROCESS
                    && uptr->ut_type == USER_PROCESS
#endif
                    ) {
                        num_users++;
                }
	}
	endutent();
        return (char *) &num_users;
}

static utmp_array *
do_names_3(int all)
{
        static utmp_array ut;
	struct utmp *uptr;
        int nusers = 0;
        
	(void)all;

        memset(&ut, 0, sizeof(ut));
        ut.utmp_array_val = &utmps[0];

        /* only use entries with both name and line fields */
        
	setutent();
        while ((uptr = getutent())!=NULL && nusers < MAXUSERS) {
                if (*uptr->ut_name && *uptr->ut_line &&
		    strncmp(uptr->ut_name, IGNOREUSER,
                            sizeof(uptr->ut_name))
#ifdef USER_PROCESS
                    && uptr->ut_type == USER_PROCESS
#endif
                    ) {
                        utmps[nusers].ut_type = RUSERS_USER_PROCESS;
                        utmps[nusers].ut_time =
                                uptr->UT_TIME;
                        utmps[nusers].ut_idle =
                                getidle(uptr->ut_line, uptr->ut_host);
                        utmps[nusers].ut_line = line[nusers];
                        strncpy(line[nusers], uptr->ut_line, 
				sizeof(line[nusers]));
			line[nusers][sizeof(line[nusers])-1] = 0;
                        utmps[nusers].ut_user = name[nusers];
                        strncpy(name[nusers], uptr->ut_name, 
				sizeof(name[nusers]));
			name[nusers][sizeof(name[nusers])-1] = 0;
                        utmps[nusers].ut_host = host[nusers];
                        strncpy(host[nusers], uptr->ut_host, 
				sizeof(host[nusers]));
			host[nusers][sizeof(host[nusers])-1] = 0;

			line[nusers][UT_LINESIZE] = '\0';
			name[nusers][UT_NAMESIZE] = '\0';
			host[nusers][UT_HOSTSIZE] = '\0';

                        nusers++;
                }
	}
        ut.utmp_array_len = nusers;

	endutent();
        return &ut;
}

utmp_array *
rusersproc_names_3(void *tmp1, CLIENT *tmp2)
{
	(void)tmp1;
	(void)tmp2;
        return do_names_3(0);
}

utmp_array *
rusersproc_allnames_3(void *tmp1, CLIENT *tmp2)
{
	(void)tmp1;
	(void)tmp2;
        return do_names_3(1);
}

static void do_strncpy(char *tgt, const char *src, size_t len) {
    strncpy(tgt, src, len);
    tgt[len-1] = 0;
}

static 
struct utmpidlearr *
do_names_2(int all)
{
        static struct utmpidlearr ut;
	struct utmp *uptr;
        int nusers = 0;
        
	(void)all;

        memset(&ut, 0, sizeof(ut));
        ut.uia_arr = utmp_idlep;
        ut.uia_cnt = 0;
        
        /* only use entries with both name and line fields */

	setutent();
        while ((uptr = getutent())!=NULL && nusers < MAXUSERS) {
                if (*uptr->ut_name && *uptr->ut_line &&
		    strncmp(uptr->ut_name, IGNOREUSER,
                            sizeof(uptr->ut_name))
#ifdef USER_PROCESS
                    && uptr->ut_type == USER_PROCESS
#endif
                    ) {
                        utmp_idlep[nusers] = &utmp_idle[nusers];
                        utmp_idle[nusers].ui_utmp.ut_time =
                                uptr->UT_TIME;
                        utmp_idle[nusers].ui_idle =
                                getidle(uptr->ut_line, uptr->ut_host);
                        do_strncpy(utmp_idle[nusers].ui_utmp.ut_line, 
				uptr->ut_line, 
				sizeof(utmp_idle[nusers].ui_utmp.ut_line));
			/* note NOT do_strncpy */
                        strncpy(utmp_idle[nusers].ui_utmp.ut_name, 
				uptr->ut_name, 
				sizeof(utmp_idle[nusers].ui_utmp.ut_name));
                        do_strncpy(utmp_idle[nusers].ui_utmp.ut_host, 
				uptr->ut_host, 
				sizeof(utmp_idle[nusers].ui_utmp.ut_host));
                        nusers++;
                }
	}

        ut.uia_cnt = nusers;
	endutent();
        return(&ut);
}

static
char *
rusersproc_names_2(void)
{
        return (char *) do_names_2(0);
}

static
char *
rusersproc_allnames_2(void)
{
        return (char *) do_names_2(1);
}

void
rusers_service(struct svc_req *rqstp, SVCXPRT *transp)
{
	union {
		int fill;
	} argument;
	char *result;
	bool_t (*xdr_argument)(void);
	xdrproc_t xdr_result;
	typedef char *(*localproc_t)(void *, struct svc_req *);
	localproc_t local;

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void)svc_sendreply(transp, (xdrproc_t) xdr_void, NULL);
		goto leave;

	case RUSERSPROC_NUM:
		xdr_argument = xdr_void;
		xdr_result = (xdrproc_t) xdr_int;
                switch (rqstp->rq_vers) {
                case RUSERSVERS_3:
                case RUSERSVERS_IDLE:
                        local = rusers_num;
                        break;
                default:
                        svcerr_progvers(transp, RUSERSVERS_IDLE, RUSERSVERS_3);
                        goto leave;
                }
		break;

	case RUSERSPROC_NAMES:
		xdr_argument = xdr_void;
		xdr_result = (xdrproc_t) xdr_utmp_array;
                switch (rqstp->rq_vers) {
                case RUSERSVERS_3:
                        local = (localproc_t) rusersproc_names_3;
                        break;

                case RUSERSVERS_IDLE:
                        xdr_result = (xdrproc_t) xdr_utmpidlearr;
                        local = (localproc_t) rusersproc_names_2;
                        break;

                default:
                        svcerr_progvers(transp, RUSERSVERS_IDLE, RUSERSVERS_3);
                        goto leave;
                        /*NOTREACHED*/
                }
		break;

	case RUSERSPROC_ALLNAMES:
		xdr_argument = xdr_void;
		xdr_result = (xdrproc_t) xdr_utmp_array;
                switch (rqstp->rq_vers) {
                case RUSERSVERS_3:
                        local = (localproc_t) rusersproc_allnames_3;
                        break;

                case RUSERSVERS_IDLE:
                        xdr_result = (xdrproc_t) xdr_utmpidlearr;
                        local = (localproc_t) rusersproc_allnames_2;
                        break;

                default:
                        svcerr_progvers(transp, RUSERSVERS_IDLE, RUSERSVERS_3);
                        goto leave;
                        /*NOTREACHED*/
                }
		break;

	default:
		svcerr_noproc(transp);
		goto leave;
	}
	memset(&argument, 0, sizeof(argument));
	if (!svc_getargs(transp, (xdrproc_t)xdr_argument, (char *)&argument)) {
		svcerr_decode(transp);
		goto leave;
	}
	result = (*local)(&argument, rqstp);
	if (result != NULL && 
	    !svc_sendreply(transp, xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, (xdrproc_t)xdr_argument, (char *)&argument)) {
		(void)fprintf(stderr, "unable to free arguments\n");
		exit(1);
	}
leave:
        if (from_inetd)
                exit(0);
}
