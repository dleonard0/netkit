all: rusers

include ../MCONFIG
include ../MRULES

ifeq ($(USE_GLIBC),1)
CFLAGS += -DGNU_LIBC -D_GNU_SOURCE -D_NO_UT_TIME
endif

RPCGEN=rpcgen

# Warning, do NOT put this in the current directory without updating
# the clean target.
RUSERSX = /usr/include/rpcsvc/rusers.x

# The rusers.h file in /usr/include/rpcsvc (at least on my system) appears
# to have been built with an ancient rpcgen. Therefore, make it anew.

rusers: rusers.o rusers_xdr.o 
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rusers
	install -s -m$(BINMODE) rusers $(INSTALLROOT)$(BINDIR)
	install -m$(MANMODE) rusers.1 $(INSTALLROOT)$(MANDIR)/man1

clean:
	rm -f *.o rusers rusers.h rusers_xdr.c rusers.x

rusers.o: rusers.h ../version.h
rusers_xdr.o: rusers_xdr.c rusers.h

# rpcgen includes the pathname you specify for the .x file as the
# pathname of the .h file when it builds the .c file. Therefore,
# do it in the current directory. 

rusers.x:
	ln -s $(RUSERSX) rusers.x

rusers.h: $(RUSERSX) rusers.x
	$(RPCGEN) -h -o rusers.h rusers.x

rusers_xdr.c: $(RUSERSX) rusers.x
	$(RPCGEN) -c -C -o rusers_xdr.c rusers.x

