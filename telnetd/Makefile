all: telnetd

include ../MCONFIG
include ../MRULES

# -DAUTHENTICATE

CFLAGS += '-DISSUE_FILE="/etc/issue.net"' -I.. \
	-DTERMCAP -DUSE_TERMIO -DKLUDGELINEMODE -DDIAGNOSTICS
LIBS += -lbsd -ltermcap

OBJS = telnetd.o state.o termstat.o slc.o sys_term.o utility.o \
	global.o authenc.o logwtmp.o

telnetd: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -L../libtelnet -ltelnet -o $@

$(OBJS): defs.h ext.h pathnames.h telnetd.h

install: telnetd
	install -s -m744 telnetd /usr/sbin/in.telnetd
	install -m644 issue.net.5 /usr/man/man5/
	install -m644 telnetd.8 /usr/man/man8/in.telnetd.8

clean:
	rm -f *.o telnetd 

