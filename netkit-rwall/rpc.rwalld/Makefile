all: rwalld

include ../MCONFIG
include ../MRULES

RPCGEN=rpcgen

rwalld: rwalld.o daemon.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

rwalld.o: rwall.h daemon.h ../version.h
daemon.o: daemon.h

install: rwalld
	install -s -m$(DAEMONMODE) rwalld $(INSTALLROOT)$(SBINDIR)/rpc.rwalld
	install -m$(MANMODE) rpc.rwalld.8 $(INSTALLROOT)$(MANDIR)/man8
	ln -sf rpc.rwalld.8 $(INSTALLROOT)$(MANDIR)/man8/rwalld.8

clean:
	rm -f *.o rwalld rwall.h

rwall.h: ../include/rpcsvc/rwall.x
	$(RPCGEN) -h -o $@ $<
