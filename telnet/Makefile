all: telnet

include ../MCONFIG
include ../MRULES

# -DAUTHENTICATE
CFLAGS += -DUSE_TERMIO -DKLUDGELINEMODE
LIBS += $(LIBTERMCAP)

SRCS = commands.cc main.cc network.cc ring.cc sys_bsd.cc telnet.cc \
	terminal.cc tn3270.cc utilities.cc genget.cc environ.cc netlink.cc

OBJS = $(patsubst %.cc, %.o, $(SRCS))

telnet: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

include .depend
.depend: 
	touch .depend
	$(MAKE) depend
depend:
	$(CC) $(CFLAGS) -MM $(SRCS) >.depend

install: telnet
	install -s telnet /usr/bin
	install -m644 telnet.1 /usr/man/man1

clean:
	rm -f *.o telnet

