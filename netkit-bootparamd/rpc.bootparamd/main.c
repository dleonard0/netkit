#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <getopt.h>
#include <unistd.h>
#include "bootparam_prot.h"


#ifdef __GLIBC__
	/* quick fix */
	void get_myaddress(struct sockaddr_in *);
#endif

int debug = 0;
int dolog = 0;
struct in_addr route_addr;
const char *bootpfile = "/etc/bootparams";

static char *progname;
static struct sockaddr_in my_addr;
static int route_addr_ok;

int
main(int argc, char **argv)
{
    SVCXPRT *transp;
    int s, pid;
    struct hostent *he;
    struct stat buf;
    char c;
    
    progname = rindex(argv[0],'/');
    if (progname) progname++;
    else progname = argv[0];

    while ((c = getopt(argc, argv,"dsr:f:")) != EOF) {
	switch (c) {
	 case 'd':
	    debug = 1;
	    break;
	 case 'r':
	    if (isdigit(*optarg)) {
		route_addr_ok = inet_aton(optarg, &route_addr);
		break;
	    } 
	    else {
		he = gethostbyname(optarg);
		if (he) {
		    memcpy(&route_addr, he->h_addr, sizeof(route_addr));
		    route_addr_ok = 1;
		    break;
		} 
		else {
		    fprintf(stderr,"%s: No such host %s\n", progname, optarg);
		    exit(1);
		}
	    }
	 case 'f':
	    bootpfile = optarg;
	    break;
	 case 's':
	    dolog = 1;
#ifndef LOG_DAEMON 
	    openlog(progname, 0 , 0);
#else
	    openlog(progname, 0 , LOG_DAEMON);
	    setlogmask(LOG_UPTO(LOG_NOTICE));
#endif
	    break;
	 default:
	    fprintf(stderr, 
		    "Usage: %s [-d] [-s] [-r router] [-f bootparmsfile]\n", 
		    argv[0]);
	    exit(1);
	}
    }
    
    if (stat(bootpfile, &buf)) {
	fprintf(stderr,"%s: ", progname);
	perror(bootpfile);
	exit(1);
    }
    

    if (!route_addr_ok) {
	get_myaddress(&my_addr);
	memcpy(&route_addr, &my_addr.sin_addr.s_addr, sizeof(route_addr));
    }
    
    if (!debug) {
	pid = fork();
	if (pid < 0) {
	    perror("bootparamd: fork");
	    exit(1);
	}
	if (pid) exit(0); /* parent */
	
	/* child */
	for (s=0; s<20; s++) close(s);
	open("/", 0);
	dup2(0, 1);
	dup2(0, 2);
	s = open("/dev/tty",2);
	if ( s >= 0 ) {
	    ioctl(s, TIOCNOTTY, 0);
	    close(s);
	}
    }
	    

    (void)pmap_unset(BOOTPARAMPROG, BOOTPARAMVERS);
    
    transp = svcudp_create(RPC_ANYSOCK);
    if (transp == NULL) {
	(void)fprintf(stderr, "cannot create udp service.\n");
	exit(1);
    }
    if (!svc_register(transp, BOOTPARAMPROG, BOOTPARAMVERS, 
		      bootparamprog_1, IPPROTO_UDP)) 
    {
	fprintf(stderr, 
		"unable to register (BOOTPARAMPROG, BOOTPARAMVERS, udp).\n");
	exit(1);
    }

    svc_run();
    (void)fprintf(stderr, "svc_run returned\n");
    exit(1);
}
