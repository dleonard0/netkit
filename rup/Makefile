all: rup

include ../MCONFIG
include ../MRULES

rup: rup.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rup
	install -s -m$(BINMODE) rup $(BINDIR)
	install -m$(MANMODE) rup.1 $(MANDIR)/man1

clean:
	rm -f *.o rup

