all: rwalld

include ../MCONFIG
include ../MRULES

RPCGEN=../rpcgen/rpcgen

rwalld: rwalld.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -lbsd -o $@

rwalld.o: rwall.h

install: rwalld
	install -s -m744 rwalld /usr/sbin/rpc.rwalld
	install -m644 rpc.rwalld.8 /usr/man/man8

clean:
	rm -f *.o rwalld rwall.h

rwall.h: ../include/rpcsvc/rwall.x
	$(RPCGEN) -h -o $@ $<
