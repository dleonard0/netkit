all: rlogin

include ../MCONFIG
include ../MRULES

ifeq ($(USE_GLIBC),1)
CFLAGS += -DGNU_LIBC -D_GNU_SOURCE
endif

OBJS=rlogin.o

rlogin: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rlogin
	install -s -o root -m$(SUIDMODE) rlogin $(INSTALLROOT)$(BINDIR)
	install -m$(MANMODE) rlogin.1 $(INSTALLROOT)$(MANDIR)/man1

clean:
	rm -f *.o rlogin
