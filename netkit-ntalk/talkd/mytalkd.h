/* 
 * Linux libc doesn't have osockaddr, but gnu libc does.
 * This has to come before include of protocols/talkd.h.
 */
#ifndef GNU_LIBC
#define osockaddr sockaddr
#endif

#include <protocols/talkd.h>
