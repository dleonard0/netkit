all: routed-query

include ../MCONFIG
include ../MRULES

OBJS=	query.o

routed-query: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: routed-query
	install -o root -g root -m $(BINMODE) routed-query $(INSTALLROOT)$(SBINDIR)
	install -o root -g root -m $(MANMODE) routed-query.8.gz $(INSTALLROOT)$(MANDIR)/man8

clean:
	rm -f $(OBJS) routed-query

