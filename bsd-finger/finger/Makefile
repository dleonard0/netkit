all: finger

include ../MCONFIG
include ../MRULES

ifeq ($(USE_GLIBC),1)
CFLAGS += -DGNU_LIBC -D_GNU_SOURCE
endif

finger: finger.o lprint.o net.o sprint.o util.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

finger.o lprint.o net.o sprint.o util.o: finger.h

install: finger
	install -s -m$(BINMODE) finger $(INSTALLROOT)$(BINDIR)
	install -m$(MANMODE) finger.1 $(INSTALLROOT)$(MANDIR)/man1

clean:
	rm -f *.o finger

