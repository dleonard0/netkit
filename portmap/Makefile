# @(#) Makefile 1.4 93/11/21 17:36:53

####################################
### Beginning of configurable stuff.

# By default, logfile entries are written to the same file as used for
# sendmail transaction logs. Change the definition of the following macro
# if you disagree. See `man 3 syslog' for examples. Some syslog versions
# do not provide this flexibility.
#
FACILITY=LOG_DAEMON

# To disable host access control, comment out the following macro definition.
# Note: host access control requires the strtok() and strchr() routines.
# Host access control can also be turned off by providing no access control
# tables. The local system, since it runs the portmap daemon, is always
# treated as an authorized host.

HOSTS_ACCESS= -DHOSTS_ACCESS

# Comment out if your RPC library does not allocate privileged ports for
# requests from processes with root privilege, or the new portmap will
# always reject requests to register/unregister services on privileged
# ports. You can find out by running "rpcinfo -p"; if all mountd and NIS
# daemons use a port >= 1024 you should probably disable the next line.

CHECK_PORT = -DCHECK_PORT

# When the portmapper cannot find any local interfaces (it will complain
# to the syslog daemon) your system probably has variable-length socket
# address structures (struct sockaddr has a sa_len component; example:
# 4.4BSD). Uncomment next macro definition in that case.
#
# SA_LEN = -DHAS_SA_LEN

# With verbose logging on, HP-UX 9.x leaves zombies behind when SIGCHLD
# is not ignored. Enable next macro for a fix.
#
ZOMBIES = -DIGNORE_SIGCHLD

# Uncomment the following macro if your system does not have u_long.
#
# ULONG	=-Du_long="unsigned long"

# Later versions of the tcp wrapper (log_tcp package) come with a
# libwrap.a object library. WRAP_DIR should specify the directory with
# that library.

# WRAP_DIR= ../tcp_wrappers-6.3

# Auxiliary object files that may be missing from your C library.
#
AUX	= # daemon.o strerror.o

# Uncomment the following macro definitions if you are running SYSV.4.
# This won't help much because SYSV.4 uses rpcbind instead of portmap.
#
# CC	= /usr/ucb/cc
# LIBS	= -lrpcsoc
# SYS	= -DSYSV40

# Auxiliary libraries that you may have to specify
#
LIBS	= -lbsd

### End of configurable stuff.
##############################

SHELL	= /bin/sh

COPT	= -Dperror=xperror $(HOSTS_ACCESS) $(CHECK_PORT) \
	$(SYS) -DFACILITY=$(FACILITY) $(ULONG) $(ZOMBIES) $(SA_LEN)
CFLAGS	= $(COPT) $(O) -include /usr/include/bsd/bsd.h -I/usr/include/bsd
OBJECTS	= portmap.o pmap_check.o from_local.o $(AUX)

all:	portmap pmap_dump pmap_set

install: all
	install -s -m 744 portmap /usr/sbin/
	install -s -m 744 pmap_dump /usr/sbin/
	install -s -m 744 pmap_set /usr/sbin/
	install -m 644 portmap.8 /usr/man/man8/

portmap: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) -lwrap $(LIBS)

pmap_dump: pmap_dump.c
	$(CC) $(CFLAGS) -o $@ $? $(LIBS)

pmap_set: pmap_set.c
	$(CC) $(CFLAGS) -o $@ $? $(LIBS)

from_local: from_local.c
	cc $(CFLAGS) -DTEST -o $@ from_local.c

lint:	
	lint $(COPT) $(OBJECTS:%.o=%.c)

clean:
	rm -f *.o portmap pmap_dump pmap_set from_local core

shar:
	@shar README Makefile portmap.c daemon.c strerror.c diffs_wrt_bsd \
	pmap_dump.c pmap_set.c from_local.c pmap_check.c pmap_check.h BLURB

diffs_wrt_bsd: portmap.c.bsd portmap.c daemon.c.bsd daemon.c
	-(diff -c portmap.c.bsd portmap.c; diff -c daemon.c.bsd daemon.c) >$@

deps:
	@$(CC) -M $(CFLAGS) *.c | grep -v /usr/include |sed 's/\.\///'

daemon.o: daemon.c
from_local.o: from_local.c
pmap_check.o: pmap_check.c pmap_check.h Makefile
pmap_dump.o: pmap_dump.c
pmap_set.o: pmap_set.c
portmap.o: portmap.c
portmap.o: pmap_check.h
strerror.o: strerror.c
