all: rwalld

include ../MCONFIG
include ../MRULES

RPCGEN=../rpcgen/rpcgen

rwalld: rwalld.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -lbsd -o $@

rwalld.o: rwall.h

install: rwalld
	install -s -m$(DAEMONMODE) rwalld $(SBINDIR)/rpc.rwalld
	install -m$(MANMODE) rpc.rwalld.8 $(MANDIR)/man8
	ln -sf rpc.rwalld.8 $(MANDIR)/man8/rwalld.8

clean:
	rm -f *.o rwalld rwall.h

rwall.h: ../include/rpcsvc/rwall.x
	$(RPCGEN) -h -o $@ $<
