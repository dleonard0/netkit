/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *
 *	@(#)err.h	8.1 (Berkeley) 6/2/93
 *	NetBSD: err.h,v 1.11 1994/10/26 00:55:52 cgd Exp
 *      From OpenBSD-current 1997/04/05 22:00 GMT
 */

#ifndef _ERR_H_
#define	_ERR_H_

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __PFX(a,b)     __attribute__((noreturn, format (printf, a, b)))
#define __PF(a,b)     __attribute__((format (printf, a, b)))

void err(int, const char *, ...)       __PFX(2,3);
void verr(int, const char *, va_list)  __PFX(2,0);
void errx(int, const char *, ...)      __PFX(2,3);
void verrx(int, const char *, va_list) __PFX(2,0);

void warn(const char *, ...)           __PF(1,2);
void vwarn(const char *, va_list)      __PF(1,0);
void warnx(const char *, ...)          __PF(1,2);
void vwarnx(const char *, va_list)     __PF(1,0);

#undef __PFX
#undef __PF

#ifdef __cplusplus
};
#endif

#endif /* !_ERR_H_ */
