all: rusersd
include ../MCONFIG
include ../MRULES

ifeq ($(USE_GLIBC),1)
CFLAGS += -DGNU_LIBC -D_GNU_SOURCE -D_NO_UT_TIME
endif


#  .if exists(/usr/include/X11/extensions/xidle.h)
#CFLAGS+= -DXIDLE
#LDADD+= -L/usr/X386/lib -lXext -lX11
#  .endif

RPCGEN=rpcgen

# Warning, do not put this in the current directory without updating
# the clean target.
RUSERSX=/usr/include/rpcsvc/rusers.x

rusersd: rusersd.o rusers_proc.o rusers_xdr.o daemon.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rusersd
	install -s -m$(DAEMONMODE) rusersd $(INSTALLROOT)$(SBINDIR)/rpc.rusersd
	install -m$(MANMODE) rpc.rusersd.8 $(INSTALLROOT)$(MANDIR)/man8
	ln -sf rpc.rusersd.8 $(INSTALLROOT)$(MANDIR)/man8/rusersd.8

clean:
	rm -f *.o rusersd rusers.h rusers_xdr.c rusers.x

rusersd.o: ../version.h
rusersd.o rusers_proc.o: rusers.h
rusers_xdr.o: rusers_xdr.c rusers.h

# see rusers makefile for an explanation of why this is necessary
rusers.x: 
	ln -s $(RUSERSX) rusers.x

rusers.h: $(RUSERSX) rusers.x
	$(RPCGEN) -h -o rusers.h rusers.x

rusers_xdr.c: $(RUSERSX)
	$(RPCGEN) -c -C -o rusers_xdr.c rusers.x
