CFLAGS += '-DISSUE_FILE="/etc/issue.net"' -DAUTHENTICATE \
	-DTERMCAP -DUSE_TERMIO -DKLUDGELINEMODE -DDIAGNOSTICS

OBJS =	auth.o enc_des.o encrypt.o genget.o getent.o \
	kerberos.o kerberos5.o misc.o

libtelnet.a: $(OBJS)
	ar rcv libtelnet.a $(OBJS)
	ranlib libtelnet.a

install: libtelnet.a
	echo "Nothing to install for libtelnet, just a libary."

clean:
	rm -f *.o libtelnet.a

