all: rcp

include ../MCONFIG
include ../MRULES

ifeq ($(USE_GLIBC),1)
CFLAGS += -DGNU_LIBC -D_GNU_SOURCE
endif

rcp: rcp.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rcp
	install -s -o root -m$(SUIDMODE) rcp $(INSTALLROOT)$(BINDIR)
	install -m$(MANMODE) rcp.1 $(INSTALLROOT)$(MANDIR)/man1

clean:
	rm -f *.o rcp

