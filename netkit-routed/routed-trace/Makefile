all: routed-trace

include ../MCONFIG
include ../MRULES

OBJS=	trace.o

routed-trace: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: routed-trace
	install -o root -g root -m $(BINMODE) routed-trace $(INSTALLROOT)$(SBINDIR)
	install -o root -g root -m $(MANMODE) routed-trace.8.gz $(INSTALLROOT)$(MANDIR)/man8

clean:
	rm -f $(OBJS) routed-trace

