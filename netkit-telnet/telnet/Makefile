all: telnet

include ../MCONFIG
include ../MRULES

ifeq ($(USE_GLIBC),1)
CFLAGS += -D_GNU_SOURCE
endif

#CFLAGS:=$(patsubst -O2, -g, $(CFLAGS))

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
	install -s -m$(BINMODE) telnet $(INSTALLROOT)$(BINDIR)
	install -m$(MANMODE) telnet.1 $(INSTALLROOT)$(MANDIR)/man1

clean:
	rm -f *.o telnet

