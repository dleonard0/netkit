all: biff

include ../MCONFIG
include ../MRULES

biff: biff.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: biff
	install -s -m$(BINMODE) biff $(BINDIR)
	install -m$(MANMODE) biff.1 $(MANDIR)/man1

clean:
	rm -f *.o biff

