/*
 * Copyright 1998 David A. Holland.
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
 * 3. Neither the name of the copyright holder(s) nor the names of their
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

char repairs_rcsid[] = 
  "$Id: repairs.c,v 1.2 1998/11/27 07:58:47 dholland Exp $";

/*
 * Most, but not quite all, of the voodoo for detecting and handling
 * malformed packets goes here.
 *
 * Basically anything which we can detect by examining the packet we
 * try to do in rationalize_packet(). We return a quirk code for what
 * we did so we can send back replies that will make sense to the other
 * end. That's irrationalize_reply().
 */

#include <sys/types.h>
#include "prot_talkd.h"
#include "proto.h"


u_int32_t
byte_swap32(u_int32_t k)
{
	return (k<<24) | ((k&0xff00) << 8) | ((k>>8) & 0xff00)  | (k>>24);
}

#if 0
static u_int16_t
byte_swap16(u_int16_t k)
{
	return (k<<8) | (k>>8);
}
#endif

/*
 * Return 0 if the packet's normal, -1 if we can't figure it out,
 * otherwise a quirk code from the quirk list.
 *
 * For now, we don't support any quirks. Need more data.
 */
int
rationalize_packet(char *buf, size_t len, struct sockaddr_in *sn)
{
	(void)buf;
	(void)sn;
	if (len == sizeof(CTL_MSG)) {
		return 0;
	}
	return -1;
}

size_t
irrationalize_reply(char *buf, size_t maxlen, int quirk)
{
	(void)buf;
	(void)maxlen;
	(void)quirk;
	return sizeof(CTL_RESPONSE);
}
