all: rusers

include ../MCONFIG
include ../MRULES

/* Doesn't compile cleanly with -Wall */
CFLAGS = -O2 -I ../include

RPCGEN = ../rpcgen.new/rpcgen

rusers: rusers.o rusers_xdr.o rnusers_xdr.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rusers
	install -s rusers /usr/bin/
	install -m644 rusers.1 /usr/man/man1

clean:
	rm -f *.o rusers rusers.h rusers_xdr.c rnusers.h rnusers_xdr.c

rusers_xdr.o: rusers_xdr.c rusers.h

rusers.h: ../include/rpcsvc/rusers.x
	$(RPCGEN) -h -o rusers.h ../include/rpcsvc/rusers.x

rusers_xdr.c: ../include/rpcsvc/rusers.x
	$(RPCGEN) -c -C -o rusers_xdr.c ../include/rpcsvc/rusers.x

rnusers_xdr.o: rnusers_xdr.c rnusers.h

rnusers.h: ../include/rpcsvc/rnusers.x
	$(RPCGEN) -h -o rnusers.h ../include/rpcsvc/rnusers.x

rnusers_xdr.c: ../include/rpcsvc/rnusers.x
	$(RPCGEN) -c -C -o rnusers_xdr.c ../include/rpcsvc/rnusers.x

