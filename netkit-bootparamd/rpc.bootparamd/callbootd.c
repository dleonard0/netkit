#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include "bootparam_prot.h"

/* #define bp_address_u bp_address */
#include <stdio.h>

#include "../version.h"
const char callbootd_rcsid[] = 
  "$Id: callbootd.c,v 1.5 1997/09/23 08:39:26 dholland Exp $";

static int printgetfile(bp_getfile_res *res);
static int printwhoami(bp_whoami_res *res);

int broadcast;

char cln[MAX_MACHINE_NAME+1];
char dmn[MAX_MACHINE_NAME+1];
char path[MAX_PATH_LEN+1];

static int
eachres_whoami(bp_whoami_res *resultp, struct sockaddr_in *raddr)
{
    struct hostent *he;

    he = gethostbyaddr((char*)&raddr->sin_addr.s_addr,4,AF_INET);
    printf("%s answered:\n", he ? he->h_name : inet_ntoa(raddr->sin_addr));
    printwhoami(resultp);
    printf("\n");
    return 0;
}

static int
eachres_getfile(bp_getfile_res *resultp, struct sockaddr_in *raddr)
{
    struct hostent *he;

    he = gethostbyaddr((char*)&raddr->sin_addr.s_addr,4,AF_INET);
    printf("%s answered:\n", he ? he->h_name : inet_ntoa(raddr->sin_addr));
    printgetfile(resultp);
    printf("\n");
    return 0;
}

int
main(int argc, char **argv)
{
    char *server;
  
    bp_whoami_arg whoami_arg;
    bp_whoami_res *whoami_res, stat_whoami_res;
    bp_getfile_arg getfile_arg;
    bp_getfile_res *getfile_res, stat_getfile_res;
    
  
    struct in_addr the_inet_addr;
    CLIENT *clnt = NULL;
    enum clnt_stat clnt_stat;

    stat_whoami_res.client_name = cln;
    stat_whoami_res.domain_name = dmn;

    stat_getfile_res.server_name = cln;
    stat_getfile_res.server_path = path;
  
    if (argc < 3) {
	fprintf(stderr,
		"Usage: %s server procnum (IP-addr | host fileid)\n", argv[0]);
	exit(1);
    } 

    
    server = argv[1];
    if (!strcmp(server , "all") ) broadcast = 1;
    
    if (!broadcast) {
	clnt = clnt_create(server,BOOTPARAMPROG, BOOTPARAMVERS, "udp");
    } 

    switch (argc) {
     case 3:
	 whoami_arg.client_address.address_type = IP_ADDR_TYPE;
	 if (!inet_aton(argv[2], &the_inet_addr)) {
	     fprintf(stderr, "bogus addr %s\n", argv[2]);
	     exit(1);
	 }
	 memcpy(&whoami_arg.client_address.bp_address_u.ip_addr, 
		&the_inet_addr, 4);
	 if (!broadcast) {
	     whoami_res = bootparamproc_whoami_1(&whoami_arg, clnt);
	     printf("Whoami returning:\n");
	     if (printwhoami(whoami_res)) {
		 fprintf(stderr, "Bad answer returned from server %s\n", 
			 server);
		 exit(1);
	     } 
	     else exit(0);
	 } 
	 else {
	     clnt_stat=clnt_broadcast(BOOTPARAMPROG, BOOTPARAMVERS, 
				      BOOTPARAMPROC_WHOAMI,
				      (xdrproc_t) xdr_bp_whoami_arg, 
				      (void *) &whoami_arg, 
				      (xdrproc_t) xdr_bp_whoami_res, 
				      (void *) &stat_whoami_res,
				      (resultproc_t) eachres_whoami);
	     exit(0);
	 }
	 
     case 4:

	 getfile_arg.client_name = argv[2];
	 getfile_arg.file_id = argv[3];
	 
	 if (!broadcast) {
	     getfile_res = bootparamproc_getfile_1(&getfile_arg,clnt);
	     printf("getfile returning:\n");
	     if (printgetfile(getfile_res)) {
		 fprintf(stderr, "Bad answer returned from server %s\n", 
			 server);
		 exit(1);
	     } 
	     else exit(0);	
	 } 
	 else {
	     clnt_stat=clnt_broadcast(BOOTPARAMPROG, BOOTPARAMVERS, 
				      BOOTPARAMPROC_GETFILE,
				      (xdrproc_t) xdr_bp_getfile_arg, 
				      (void *) &getfile_arg, 
				      (xdrproc_t) xdr_bp_getfile_res, 
				      (void *) &stat_getfile_res,
				      (resultproc_t) eachres_getfile);
	     exit(0);
	 }
	 
     default:
	 fprintf(stderr, "Usage: %s server procnum (IP-addr | host fileid)\n", 
		 argv[0]);
	 exit(1);
    }
}


int printwhoami(bp_whoami_res *res)
{
    if (res) {
	printf("client_name:\t%s\ndomain_name:\t%s\n",
	       res->client_name, res->domain_name);
	printf("router:\t%d.%d.%d.%d\n",
	       255 & res->router_address.bp_address_u.ip_addr.net,
	       255 & res->router_address.bp_address_u.ip_addr.host,
	       255 & res->router_address.bp_address_u.ip_addr.lh,
	       255 & res->router_address.bp_address_u.ip_addr.impno);
	return 0;
    } 
    else {
	fprintf(stderr,"Null answer!!!\n");
	return 1;
    }
}

static int
printgetfile(bp_getfile_res *res)
{
    if (res) {
	struct in_addr sia;
	memcpy(&sia, &res->server_address.bp_address_u.ip_addr, 4);
	printf("server_name:\t%s\nserver_address:\t%s\npath:\t%s\n",
	       res->server_name, 
	       inet_ntoa(sia),
	       res->server_path);
	return 0;
      } 
    else {
	fprintf(stderr,"Null answer!!!\n");
	return(1);
    }
}
