all: timed

include ../../MCONFIG
include ../../MRULES

LIBS += -L../lib -ltimed -lbsd

OBJS = timed.o acksend.o candidate.o correct.o master.o networkdelta.o \
	readmsg.o slave.o 

timed: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): globals.h pathnames.h timed-extern.h

install: timed
	install -s -m$(DAEMONMODE) timed $(SBINDIR)
	install -m$(MANMODE) timed.8 $(MANDIR)/man8

clean:
	rm -f *.o timed

