all: biff

include ../MCONFIG

%.o: %.c
	$(CC) $(CFLAGS) $< -c

biff.o: ../version.h

biff: biff.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: biff
	install -s -m$(BINMODE) biff $(INSTALLROOT)$(BINDIR)
	install -m$(MANMODE) biff.1 $(INSTALLROOT)$(MANDIR)/man1

clean:
	rm -f *.o biff

