/*
 * Modifications for Linux-PAM: Al Longyear <longyear@netcom.com>
 *   General code clean up: Andrew Morgan <morgan@physics.ucla.edu>
 *   Re-built with #ifdef USE_PAM: Michael K. Johnson <johnsonm@redhat.com>,
 *   Red Hat Software
 *
 *   The Linux-PAM mailing list (25JUN96) <pam-list@redhat.com>
 */

#ifdef USE_PAM
/* conversation function and static variables for communication */

/*
 * A generic conversation function for text based applications
 *
 * Written by Andrew Morgan <morgan@physics.ucla.edu>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <security/pam_appl.h>

#define INPUTSIZE PAM_MAX_MSG_SIZE

#define CONV_ECHO_ON  1

#define CONV_ECHO_OFF 0

#define _pam_overwrite(x) \
{ \
     register char *xx; \
     if ((xx=x)) \
          while (*xx) \
               *xx++ = '\0'; \
}

static char *read_string(int echo, const char *remark)
{
     char buffer[INPUTSIZE];
     char *text,*tmp;

     if (!echo) {
         tmp = getpass(remark);
         text = strdup(tmp);       /* get some space for this text */
         _pam_overwrite(tmp);       /* overwrite the old record of
                                     * the password */
     } else {
         fprintf(stderr,"%s",remark);
         text = fgets(buffer,INPUTSIZE-1,stdin);
         if (text) {
              tmp = buffer + strlen(buffer);
              while (tmp > buffer && (*--tmp == '\n'))
                   *tmp = '\0';
              text = strdup(buffer);  /* get some space for this text */
         }
     }

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

int misc_conv(int num_msg, const struct pam_message **msgm,
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
                   reply = ptmp;                       /* enlarged array*/
              }

              /* add string to list of responses */

              reply[replies].resp_retcode = 0;
              reply[replies++].resp = string;
              string = NULL;
         }
     }

     /* do we need to bother with a response? */
     if (reply) {

         /* note, this pam_response structure (array) will be
          * free()'d by the module */

         *response = reply;
     }

     return PAM_SUCCESS;
}

struct pam_conv conv = {
	misc_conv,
	NULL
};

#endif /* USE_PAM */

