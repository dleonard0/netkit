all: rwall

include ../MCONFIG
include ../MRULES

RPCGEN=rpcgen

rwall: rwall.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

rwall.o: rwall.h ../version.h

install: rwall
	install -s -m$(BINMODE) rwall $(INSTALLROOT)$(BINDIR)
	install -m$(MANMODE) rwall.1 $(INSTALLROOT)$(MANDIR)/man1/

clean:
	rm -f *.o rwall rwall.h

rwall.h: ../include/rpcsvc/rwall.x
	$(RPCGEN) -h -o $@ $<
