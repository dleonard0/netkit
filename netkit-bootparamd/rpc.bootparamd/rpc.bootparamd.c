#include <rpc/rpc.h>
#include "bootparam_prot.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <syslog.h>
#include <unistd.h>

extern int debug, dolog;
extern struct in_addr route_addr;
extern char *bootpfile;

static int getthefile(char *askname, char *fileid, char *buffer);
static int checkhost(char *askname, char *hostname);

#define MAXLEN 800

struct hostent *he;

static char buffer[MAXLEN];
static char hostname[MAX_MACHINE_NAME];
static char askname[MAX_MACHINE_NAME];
static char path[MAX_PATH_LEN];
static char domain_name[MAX_MACHINE_NAME];

  
bp_whoami_res *
bootparamproc_whoami_1_svc(bp_whoami_arg *whoami, struct svc_req *svc_req)
{
    long haddr;
    static bp_whoami_res res;
    
    (void)svc_req;

    if (debug) {
	fprintf(stderr,"whoami got question for %d.%d.%d.%d\n", 
		255 & whoami->client_address.bp_address_u.ip_addr.net,
		255 & whoami->client_address.bp_address_u.ip_addr.host,
		255 & whoami->client_address.bp_address_u.ip_addr.lh,
		255 & whoami->client_address.bp_address_u.ip_addr.impno);
    }
    if (dolog) {
	syslog(LOG_NOTICE, "whoami got question for %d.%d.%d.%d\n", 
	       255 & whoami->client_address.bp_address_u.ip_addr.net,
	       255 & whoami->client_address.bp_address_u.ip_addr.host,
	       255 & whoami->client_address.bp_address_u.ip_addr.lh,
	       255 & whoami->client_address.bp_address_u.ip_addr.impno);
    }

    memcpy(&haddr, &whoami->client_address.bp_address_u.ip_addr, 
	   sizeof(haddr));
    he = gethostbyaddr((char *)&haddr,sizeof(haddr),AF_INET);
    if (!he) goto failed;

    if (debug) fprintf(stderr,"This is host %s\n", he->h_name);
    if (dolog) syslog(LOG_NOTICE,"This is host %s\n", he->h_name);
    
    strcpy(askname, he->h_name);
    if (checkhost(askname, hostname)) {
	res.client_name = hostname;
	getdomainname(domain_name, MAX_MACHINE_NAME);
	res.domain_name = domain_name;
	
	if (res.router_address.address_type != IP_ADDR_TYPE) {
	    res.router_address.address_type = IP_ADDR_TYPE;
	    memcpy(&res.router_address.bp_address_u.ip_addr, &route_addr, 4);
	}
	if (debug) fprintf(stderr,
			   "Returning %s   %s    %d.%d.%d.%d\n", 
			   res.client_name,
			   res.domain_name,
			   255& res.router_address.bp_address_u.ip_addr.net,
			   255& res.router_address.bp_address_u.ip_addr.host,
			   255& res.router_address.bp_address_u.ip_addr.lh,
			   255& res.router_address.bp_address_u.ip_addr.impno);
	if (dolog) syslog(LOG_NOTICE,
			  "Returning %s   %s    %d.%d.%d.%d\n", 
			  res.client_name,
			  res.domain_name,
			  255 & res.router_address.bp_address_u.ip_addr.net,
			  255 & res.router_address.bp_address_u.ip_addr.host,
			  255 & res.router_address.bp_address_u.ip_addr.lh,
			  255 & res.router_address.bp_address_u.ip_addr.impno);
	
	return(&res);
    }
  failed:
    if (debug) fprintf(stderr,"whoami failed\n");
    if (dolog) syslog(LOG_NOTICE,"whoami failed\n");
    return NULL;
}


bp_getfile_res *
bootparamproc_getfile_1_svc(bp_getfile_arg *getfile,struct svc_req *svc_req)
{
    char *where;
    static bp_getfile_res res;
    
    (void)svc_req;
    
    if (debug) 
	fprintf(stderr,"getfile got question for \"%s\" and file \"%s\"\n",
		getfile->client_name, getfile->file_id);
    
    if (dolog) 
	syslog(LOG_NOTICE,"getfile got question for \"%s\" and file \"%s\"\n",
	       getfile->client_name, getfile->file_id);
    
    he = NULL;
    he = gethostbyname(getfile->client_name);
    if (! he ) goto failed;
    
    strcpy(askname,he->h_name);
    if (getthefile(askname, getfile->file_id,buffer)) {
	if ((where = strchr(buffer,':'))!=NULL) {
	    /* buffer is re-written to contain the name of the info of file */
	    strncpy(hostname, buffer, where - buffer);
	    hostname[where - buffer] = '\0';
	    where++;
	    strcpy(path, where);
	    he = gethostbyname(hostname);
	    if ( !he ) goto failed;
	    bcopy( he->h_addr, &res.server_address.bp_address_u.ip_addr, 4);
	    res.server_name = hostname;
	    res.server_path = path;
	    res.server_address.address_type = IP_ADDR_TYPE;
	}
	else { /* special for dump, answer with null strings */
	    if (!strcmp(getfile->file_id, "dump")) {
		res.server_name[0] = '\0';
		res.server_path[0] = '\0';	
		bzero(&res.server_address.bp_address_u.ip_addr,4);
	    } 
	    else goto failed;
	}
	if (debug) 
	    fprintf(stderr, 
		    "returning server:%s path:%s address: %d.%d.%d.%d\n",
		    res.server_name, res.server_path,
		    255 & res.server_address.bp_address_u.ip_addr.net,
		    255 & res.server_address.bp_address_u.ip_addr.host,
		    255 & res.server_address.bp_address_u.ip_addr.lh,
		    255 & res.server_address.bp_address_u.ip_addr.impno);
	if (dolog) 
	    syslog(LOG_NOTICE, 
		   "returning server:%s path:%s address: %d.%d.%d.%d\n",
		   res.server_name, res.server_path,
		   255 & res.server_address.bp_address_u.ip_addr.net,
		   255 & res.server_address.bp_address_u.ip_addr.host,
		   255 & res.server_address.bp_address_u.ip_addr.lh,
		   255 & res.server_address.bp_address_u.ip_addr.impno);
	return(&res);
    }
  failed:
    if (debug) fprintf(stderr, 
		       "getfile failed for %s\n", getfile->client_name);
    if (dolog) syslog(LOG_NOTICE, 
		      "getfile failed for %s\n", getfile->client_name);
    return NULL;
}

/*    getthefile return 1 and fills the buffer with the information
      of the file, e g "host:/export/root/client" if it can be found.
      If the host is in the database, but the file is not, the buffer 
      will be empty. (This makes it possible to give the special
      empty answer for the file "dump")   */

static int
getthefile(char *askname,char *fileid,char *buffer)
{
    FILE *bpf;
    char *where;

    int ch, pch, fid_len, res = 0;
    int match = 0;
    char info[MAX_FILEID + MAX_PATH_LEN+MAX_MACHINE_NAME + 3];
    
    bpf = fopen(bootpfile, "r");
    if (!bpf) {
	fprintf(stderr, "No %s\n", bootpfile);
	exit(1);
    }

    while (fscanf(bpf, "%s", hostname) >  0  && !match) {
	if ( *hostname != '#' ) { /* comment */
	    if (!strcmp(hostname, askname)) {
		match = 1;
	    } 
	    else {
		he = gethostbyname(hostname);
		if (he && !strcmp(he->h_name, askname)) match = 1;
	    }
	}
	/* skip to next entry */
	if (match) break;
	pch = ch = getc(bpf); 
	while ( ! ( ch == '\n' && pch != '\\') && ch != EOF) {
	    pch = ch; ch = getc(bpf);
	}
    }

    /* if match is true we read the rest of the line to get the
       info of the file */

    if (match) {
	fid_len = strlen(fileid);
	while (!res && (fscanf(bpf,"%s", info)) > 0) { /* read a string */
	    ch = getc(bpf);                            /* and a character */
	    if (*info != '#') {                        /* Comment ? */
		if (!strncmp(info, fileid, fid_len) && 
		    *(info + fid_len) == '=') 
		{
		    where = info + fid_len + 1;
		    if (isprint(*where)) {
			strcpy(buffer, where);        /* found file */
			res = 1; break;
		    }
		} 
		else {  
		    while (isspace(ch) && ch != '\n') ch = getc(bpf); 
				                     /* read to end of line */
		    if ( ch == '\n' ) {              /* didn't find it */
			res = -1; break;             /* but host is there */
		    }
		    if ( ch == '\\' ) {              /* more info */
			ch = getc(bpf);              /* maybe on next line */
			if (ch == '\n') continue;    /* read it in next loop */
			ungetc(ch, bpf); ungetc('\\',bpf); 
                                               /* push the character(s) back */
		    } else ungetc(ch, bpf);    /* but who know what a `\` is */
		}                              /* needed for. */
	    } else break;                      /* a commented rest-of-line */
	}
    }
    if (fclose(bpf)) { fprintf(stderr,"Could not close %s\n", bootpfile); }
    if ( res == -1) buffer[0] = '\0';            /* host found, file not */
    return(match);
}

/* checkhost puts the hostname found in the database file in
   the hostname-variable and returns 1, if askname is a valid
   name for a host in the database */

static int
checkhost(char *askname, char *hostname)
{
    int ch, pch;
    FILE *bpf;
    int res = 0;
    
    bpf = fopen(bootpfile, "r");
    if (!bpf) {
	fprintf(stderr, "No %s\n", bootpfile);
	exit(1);
    }

    while (fscanf(bpf, "%s", hostname) > 0) {
	if (!strcmp(hostname, askname)) {
	    /* return true for match of hostname */
	    res = 1;
	    break;
	} 
	else {
	    /* check the alias list */
	    he = NULL;
	    he = gethostbyname(hostname);
	    if (he && !strcmp(askname, he->h_name)) {
		res = 1;
		break;
	    }
	}
	/* skip to next entry */
	pch = ch = getc(bpf); 
	while (!(ch == '\n' && pch != '\\') && ch != EOF) {
	    pch = ch; ch = getc(bpf);
	}
    }
    if (fclose(bpf)) { fprintf(stderr,"Could not close %s\n", bootpfile); }
    return(res);
}
