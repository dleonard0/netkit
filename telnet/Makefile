CFLAGS += '-DISSUE_FILE="/etc/issue.net"' -DAUTHENTICATE -I.. \
	-DTERMCAP -DUSE_TERMIO -DKLUDGELINEMODE -DDIAGNOSTICS
LDLIBS = -lbsd -ltermcap -ltelnet -L ../libtelnet

telnet: authenc.o commands.o main.o network.o ring.o sys_bsd.o telnet.o \
		terminal.o tn3270.o utilities.o

install: telnet
	install -s telnet /usr/bin
	install -m644 telnet.1 /usr/man/man1

clean:
	rm -f *.o telnet

