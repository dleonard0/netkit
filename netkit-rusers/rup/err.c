/*
 * Reimplementation of err/warnx.
 */

char err_rcsid[] = 
  "$Id: err.c,v 1.1 1997/04/05 22:13:31 dholland Exp $";

#include <stdio.h>
#include <stdarg.h>
#include <err.h>

void err(int eval, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "rup: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "%m\n");
    va_end(ap);
    exit(eval);
}

void warnx(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "rup: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}
