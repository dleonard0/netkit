.EXPORT_ALL_VARIABLES:

# comment this out, if you don't have shadow passwords on your system
# This will affect the compilation of the subdirs: ftpd
HAVE_SHADOW_PASSWORDS=true

# Do you want to have GNU readline and history support in 'ftp'. Use
# cursor keys to get your last commands.
# I use the GNU readline support from bash-1.14 for this.
USE_GNU_READLINE=true

# The TCP wrapper doesn't really work for the "rexecd", since it
# waits for further incoming connections once invoked (which are not
# checked by the TCP wrapper binary).
# Install the TCP wrapper library to compile rexecd with these extra checks.
# Also, *DONT* wrap the "rexecd" in your /etc/inetd.conf!

# This may not work right now!!!!!!!!!!!!!!!!
# HAVE_TCP_WRAPPER_LIBRARY=true

# Optimization for compiling all programs.
O=-O2 -fomit-frame-pointer -pipe

# Flags for ld. You don't have to add "-s", since all binaries are
# stripped on installation ("install -s ...").
# LDFLAGS=-v
LDFLAGS=

################### END OF CONFIGURATION PART ###############################

# Just to have a short-cut in the subdirectory Makefiles.
IBSD=-I/usr/include/bsd -include /usr/include/bsd/bsd.h

# These are just a common cases. Then we don't have do write anything in the
# sublevel makefiles.
CFLAGS = ${O} ${IBSD}
LDLIBS = -lbsd

SUB = arp biff comsat finger fingerd ftp ftpd inetd libtelnet lpr \
	ping portmap rcp rexecd rlogin rlogind routed rpc.rusersd \
	rpc.rwalld rpcgen.new rpcinfo rsh rshd rusers rwall rwho rwhod \
	slattach.new sliplogin talk talkd telnet telnetd tftp tftpd timed
# Missing subdirs: rarpd, rup

all:
	for i in $(SUB); do make -C  $$i; done

install:
	for i in $(SUB); do make -C  $$i install; done

clean:
	for i in $(SUB); do make -C  $$i clean; done

