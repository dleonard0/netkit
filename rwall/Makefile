all: rwall

include ../MCONFIG
include ../MRULES

RPCGEN=../rpcgen/rpcgen

rwall: rwall.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

rwall.o: rwall.h

install: rwall
	install -s -m755 rwall /usr/bin/
	install -m644 rwall.1 /usr/man/man1/

clean:
	rm -f *.o rwall rwall.h

rwall.h: ../include/rpcsvc/rwall.x
	$(RPCGEN) -h -o $@ $<
