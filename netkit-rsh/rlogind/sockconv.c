/*
 * A generic conversation function for text based applications
 *
 * Written by Andrew Morgan <morgan@physics.ucla.edu>
 *    modified for socket file descriptors by Erik Troan <ewt@redhat.com>
 *
 * $Log: sockconv.c,v $
 * Revision 1.2  1997/06/08 19:57:22  dholland
 * minor fix - don't define __USE_BSD if already defined.
 *
 * Revision 1.1  1997/04/06 00:32:37  dholland
 * Initial revision
 *
 *
 * From: misc_conv.c,v 1.2 1996/07/07 23:59:56 morgan Exp
 *
 * Revision 1.2  1996/07/07 23:59:56  morgan
 * changed the name of the misc include file
 *
 * Revision 1.1  1996/05/02 05:17:06  morgan
 * Initial revision
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef __USE_BSD
#define __USE_BSD                /* needed for prototype for getpass() */
#endif
#include <unistd.h>

#include <security/pam_appl.h>
#include <security/pam_misc.h>

int sock_conv(int num_msg, const struct pam_message **msgm,
		     struct pam_response **response, void *appdata_ptr);

#define INPUTSIZE PAM_MAX_MSG_SIZE

#define CONV_ECHO_ON  1
#define CONV_ECHO_OFF 0

static char *read_string(int echo, const char *remark)
{
     char buffer[INPUTSIZE];
     char *text;
     int charsRead = 0;
     char * nl = "\n\r";
 
     fprintf(stderr,"%s",remark);

     while (charsRead < (INPUTSIZE - 1)) {
	  read(0, &buffer[charsRead], 1);
	 
	  if (buffer[charsRead] == '\r') {
	       write(1, nl, 2);
	       buffer[charsRead] = '\0';
	       break;
	  }

	  if (echo) {
	       write(1, &buffer[charsRead], 1);
	  }

	  charsRead++;
     }

     text = xstrdup(buffer);  /* get some space for this text */

     return (text);
}

#define REPLY_CHUNK 5

static void drop_reply(struct pam_response *reply, int replies)
{
     int i;

     for (i=0; i<replies; ++i) {
	  _pam_overwrite(reply[i].resp);      /* might be a password */
	  free(reply[i].resp);
     }
     if (reply)
	  free(reply);
}

int sock_conv(int num_msg, const struct pam_message **msgm,
		     struct pam_response **response, void *appdata_ptr)
{
     int count=0,replies=0,space=0;
     struct pam_response *reply=NULL;
     char *string=NULL;

     for (count=0; count < num_msg; ++count) {
	  switch (msgm[count]->msg_style) {
	  case PAM_PROMPT_ECHO_OFF:
	       string = read_string(CONV_ECHO_OFF,msgm[count]->msg);
	       if (string == NULL) {
		    drop_reply(reply,replies);
		    return (PAM_CONV_ERR);
	       }
	       break;
	  case PAM_PROMPT_ECHO_ON:
	       string = read_string(CONV_ECHO_ON,msgm[count]->msg);
	       if (string == NULL) {
		    drop_reply(reply,replies);
		    return (PAM_CONV_ERR);
	       }
	       break;
	  case PAM_ERROR_MSG:
	       fprintf(stderr,"%s\n",msgm[count]->msg);
	       break;
	  case PAM_TEXT_INFO:
	       fprintf(stderr,"%s\n",msgm[count]->msg);
	       break;
	  default:
	       fprintf(stderr, "erroneous conversation (%d)\n"
		       ,msgm[count]->msg_style);
	       drop_reply(reply,replies);
	       return (PAM_CONV_ERR);
	  }

	  if (string) {     /* must add to reply array */
	       struct pam_response *ptmp;

	       /* do we need a larger reply array ? */

	       if (space <= replies) {
		    space += REPLY_CHUNK;
		    ptmp = (struct pam_response *)
			 realloc(reply, space*sizeof(struct pam_response));
		    if (ptmp == NULL) {
			 drop_reply(reply,replies);
			 return PAM_CONV_ERR;        /* ran out of memory */
		    }
		    reply = ptmp;                       /* enlarged array */
	       }

	       /* add string to list of responses */

	       reply[replies].resp_retcode = 0;
	       reply[replies++].resp = string;
	       string = NULL;
	  }
     }

     /* do we need to bother with a response? */
     if (reply) {

	  /*
	   * note, this pam_response structure (array) will be
	   * free()'d by the module 
	   */

	  *response = reply;
     }
     
     return PAM_SUCCESS;
}
