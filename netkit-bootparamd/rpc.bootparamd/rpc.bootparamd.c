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
#include <assert.h>

#include "../version.h"
const char bootparamd_rcsid[] = 
  "$Id: rpc.bootparamd.c,v 1.12 2000/07/22 16:23:56 dholland Exp $";

extern int debug, dolog;
extern struct in_addr route_addr;
extern const char *bootpfile;

static int getthefile(const char *askname, const char *fileid, 
		      char *buf, size_t buflen);
static int checkhost(const char *askname, char *hostname, size_t hostsize);

#define MAXLEN 800

struct hostent *he;

static char buffer[MAXLEN];
static char hostname[MAX_MACHINE_NAME];
static char askname[MAX_MACHINE_NAME];
static char path[MAX_PATH_LEN];
static char domain_name[MAX_MACHINE_NAME];

static
const char *
my_ntoa(struct ip_addr_t addr)
{
    static char buf[20];

    /*
     * This is the same order it used to print the fields in before I 
     * combined all the printing, but I really wonder if it's right.
     */

    snprintf(buf, sizeof(buf), "%d.%d.%d.%d", 
	     addr.net&255, addr.host&255, addr.lh&255, addr.impno&255); 
    return buf;
}

bp_whoami_res *
bootparamproc_whoami_1_svc(bp_whoami_arg *whoami, struct svc_req *svc_req)
{
    static bp_whoami_res res;
    struct ip_addr_t addr;
    const char *addrname;
    
    (void)svc_req;

    addr = whoami->client_address.bp_address_u.ip_addr;
    addrname = my_ntoa(addr);

    if (debug) {
	fprintf(stderr, "bootparamd: whoami got question for %s\n", addrname);
    }
    if (dolog) {
	syslog(LOG_NOTICE, "whoami got question for %s\n", addrname);
    }

    he = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
    if (!he) goto failed;

    if (debug) fprintf(stderr,"This is host %s\n", he->h_name);
    if (dolog) syslog(LOG_NOTICE,"This is host %s\n", he->h_name);
    
    strncpy(askname, he->h_name, sizeof(askname));
    askname[sizeof(askname)-1] = 0;

    if (checkhost(askname, hostname, sizeof(hostname))) {
	res.client_name = hostname;
	getdomainname(domain_name, sizeof(domain_name));
	res.domain_name = domain_name;
	
	if (res.router_address.address_type != IP_ADDR_TYPE) {
	    res.router_address.address_type = IP_ADDR_TYPE;
	    memcpy(&res.router_address.bp_address_u.ip_addr, &route_addr, 
		   sizeof(route_addr));
	}
	if (debug) fprintf(stderr,
			   "Returning %s   %s    %s\n", 
			   res.client_name,
			   res.domain_name,
			   my_ntoa(res.router_address.bp_address_u.ip_addr));
	if (dolog) syslog(LOG_NOTICE,
			  "Returning %s   %s    %s\n", 
			  res.client_name,
			  res.domain_name,
			  my_ntoa(res.router_address.bp_address_u.ip_addr));
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
    
    he = gethostbyname(getfile->client_name);
    if (!he) goto failed;
    
    strncpy(askname, he->h_name, sizeof(askname));
    askname[sizeof(askname)-1] = 0;

    if (getthefile(askname, getfile->file_id, buffer, sizeof(buffer))) {
	if ((where = strchr(buffer,':'))!=NULL) {
	    /* buffer is re-written to contain the name of the info of file */
	    strncpy(hostname, buffer, where - buffer);
	    hostname[where - buffer] = '\0';
	    where++;
	    strncpy(path, where, sizeof(path));
	    path[sizeof(path)-1] = 0;
	    he = gethostbyname(hostname);
	    if (!he) goto failed;
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
		    "returning server:%s path:%s address: %s\n",
		    res.server_name, res.server_path,
		    my_ntoa(res.server_address.bp_address_u.ip_addr));
	if (dolog) 
	    syslog(LOG_NOTICE, 
		   "returning server:%s path:%s address: %s\n",
		   res.server_name, res.server_path,
		   my_ntoa(res.server_address.bp_address_u.ip_addr));
	return(&res);
    }
  failed:
    if (debug) fprintf(stderr, 
		       "getfile failed for %s\n", getfile->client_name);
    if (dolog) syslog(LOG_NOTICE, 
		      "getfile failed for %s\n", getfile->client_name);
    return NULL;
}

static int
get_bootparam_line(char *buf, size_t len, FILE *f)
{
	size_t pos = 0, inc;

	while (fgets(buf+pos, len-pos, f)) {
		inc = strlen(buf+pos);
		pos += inc;
		if (inc < 2 || buf[pos-1]!='\n' || buf[pos-2]!='\\') {
			break;
		}
		/* remove the escaped newline */
		pos -= 2;
		buf[pos] = 0;
	}

	return pos>0;
}

static int 
get_bootparam_entry(const char *askname, const char *capname,
		    char *hostbuf, size_t hostbuflen,
		    char *valbuf, size_t valbuflen,
		    FILE *f)
{
	char linebuf[8192];
	char *words[16], *s;
	int nwords, i;
	int found;
	size_t caplen;

	while (get_bootparam_line(linebuf, sizeof(linebuf), f)) {
		if (*linebuf=='#') continue;  /* comment */
		nwords = 0;
		for (s=strtok(linebuf, " \t\n"); s; s=strtok(NULL, " \t\n")) {
			if (s[0]=='#') break; /* comment */
			if (nwords < 16) words[nwords++] = s;
		}
		if (nwords==0) continue;  /* blank line */

		found = 0;

		if (!strcmp(askname, words[0])) {
			found = 1;
		}
		else {
			struct hostent *hp = gethostbyname(words[0]);
			if (hp && !strcmp(askname, hp->h_name)) found = 1;
		}

		if (!found) continue;

		if (hostbuf) {
			snprintf(hostbuf, hostbuflen, "%s", words[0]);
		}

		if (!capname) return 1;

		caplen = strlen(capname);
		if (valbuf) *valbuf = 0;

		for (i=1; i<nwords; i++) {
			if (!strncmp(words[i], capname, caplen) &&
			    words[i][caplen]=='=') {
				if (valbuf) {
					snprintf(valbuf, valbuflen, "%s",
						 words[i]+caplen+1);
				}
				break;
			}
		}
		/* Item not found, host is: return success and valbuf of "" */
		return 1;
	}
	return 0;
}

/*
 * getthefile returns 1 and fills the buffer with the information of
 * the file, e.g. "host:/export/root/client" if it can be found.  If
 * the host is in the database, but the file is not, the buffer will
 * be empty. (This makes it possible to give the special empty answer
 * for the file "dump")
 */

static int
getthefile(const char *askname, const char *fileid, char *buffer, size_t bufferlen)
{
    FILE *bpf;
    int match = 0;
    
    bpf = fopen(bootpfile, "r");
    if (!bpf) {
	fprintf(stderr, "No %s\n", bootpfile);
	exit(1);
    }

    match = get_bootparam_entry(askname, fileid, 
				NULL, 0,
				buffer, bufferlen,
				bpf);

    if (fclose(bpf)) { fprintf(stderr,"Could not close %s\n", bootpfile); }

    return(match);
}

/* 
 * checkhost puts the hostname found in the database file in the
 * hostname-variable and returns 1, if askname is a valid name for a
 * host in the database.
 */

static int
checkhost(const char *askname, char *hostname, size_t hostsize)
{
    FILE *bpf;
    int res = 0;
    
    bpf = fopen(bootpfile, "r");
    if (!bpf) {
	fprintf(stderr, "No %s\n", bootpfile);
	exit(1);
    }

    res = get_bootparam_entry(askname, NULL, 
			      hostname, hostsize,
			      NULL, 0,
			      bpf);

    if (fclose(bpf)) { fprintf(stderr,"Could not close %s\n", bootpfile); }
    return(res);
}
