CFLAGS += -I ../include

rusers: rusers.o rusers_xdr.o rnusers_xdr.o

install: rusers
	install -s rusers /usr/bin/
	install -m644 rusers.1 /usr/man/man1

clean:
	rm -f *.o rusers rusers.h rusers_xdr.c rnusers.h rnusers_xdr.c

rusers_xdr.o: rusers_xdr.c rusers.h

rusers.h: ../include/rpcsvc/rusers.x
	../rpcgen.new/rpcgen -h -o rusers.h ../include/rpcsvc/rusers.x

rusers_xdr.c: ../include/rpcsvc/rusers.x
	../rpcgen.new/rpcgen -c -C -o rusers_xdr.c ../include/rpcsvc/rusers.x

rnusers_xdr.o: rnusers_xdr.c rnusers.h

rnusers.h: ../include/rpcsvc/rnusers.x
	../rpcgen.new/rpcgen -h -o rnusers.h ../include/rpcsvc/rnusers.x

rnusers_xdr.c: ../include/rpcsvc/rnusers.x
	../rpcgen.new/rpcgen -c -C -o rnusers_xdr.c ../include/rpcsvc/rnusers.x

