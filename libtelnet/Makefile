all: libtelnet.a

include ../MCONFIG
include ../MRULES

# -DAUTHENTICATE

CFLAGS += '-DISSUE_FILE="/etc/issue.net"' \
	-DTERMCAP -DUSE_TERMIO -DKLUDGELINEMODE -DDIAGNOSTICS

OBJS =	genget.o getent.o


libtelnet.a: $(OBJS)
	ar cruv libtelnet.a $(OBJS)
	ranlib libtelnet.a

install: libtelnet.a
	@echo "Nothing to install for libtelnet."

clean:
	rm -f *.o libtelnet.a

