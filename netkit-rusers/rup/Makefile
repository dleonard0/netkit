all: rup

include ../MCONFIG
include ../MRULES

ifeq ($(USE_GLIBC),1)
CFLAGS += -DGNU_LIBC -D_GNU_SOURCE
endif

CFLAGS += -I.

#RPCGEN=../../netkit-base/rpcgen/rpcgen
RPCGEN=rpcgen

# Warning, do NOT put this in the current directory without updating
# the clean target.
RSTATX=/usr/include/rpcsvc/rstat.x

rup: rup.o rstat_xdr.o err.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

rup.o: ../version.h

install: rup
	install -s -m$(BINMODE) rup $(INSTALLROOT)$(BINDIR)
	install -m$(MANMODE) rup.1 $(INSTALLROOT)$(MANDIR)/man1

clean:
	rm -f *.o rup rstat.h rstat_xdr.c rstat.x

rstat_xdr.o: rstat_xdr.c rstat.h

# rpcgen includes the pathname you specify for the .x file as the
# pathname of the .h file when it builds the .c file. Therefore,
# do it in the current directory. 

rstat.x: 
	ln -s $(RSTATX) $@

rstat.h: $(RSTATX) rstat.x
	$(RPCGEN) -h -o rstat.h rstat.x

rstat_xdr.c: $(RSTATX) rstat.x
	$(RPCGEN) -c -C -o rstat_xdr.c rstat.x
