all: telnet

include ../MCONFIG
include ../MRULES

# -DAUTHENTICATE

CFLAGS += '-DISSUE_FILE="/etc/issue.net"' -I.. \
	-DTERMCAP -DUSE_TERMIO -DKLUDGELINEMODE -DDIAGNOSTICS
LIBS += -L../libtelnet -ltelnet -ltermcap 

OBJS = authenc.o commands.o main.o network.o ring.o sys_bsd.o telnet.o \
	terminal.o tn3270.o utilities.o

# Warning. If you link it with ncurses instead of libtermcap it will crash.
# This will be fixed in a future release.

telnet: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): defines.h externs.h fdset.h general.h proto.h ring.h types.h

install: telnet
	install -s telnet /usr/bin
	install -m644 telnet.1 /usr/man/man1

clean:
	rm -f *.o telnet

