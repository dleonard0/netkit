all: rusersd
include ../MCONFIG
include ../MRULES

#  .if exists(/usr/include/X11/extensions/xidle.h)
#CFLAGS+= -DXIDLE
#LDADD+= -L/usr/X386/lib -lXext -lX11
#  .endif

RPCGEN=../rpcgen.new/rpcgen

# Doesn't compile cleanly with -Wall
CFLAGS = -O2 -D__USE_BSD_SIGNAL -I../include

rusersd: rusersd.o rusers_proc.o rusers_xdr.o rnusers_xdr.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -lbsd -o $@

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

install: rusersd
	install -s -m744 rusersd /usr/sbin/rpc.rusersd
	install -m644 rpc.rusersd.8 /usr/man/man8

clean:
	rm -f *.o rusersd rusers.h rusers_xdr.c rnusers.h rnusers_xdr.c

