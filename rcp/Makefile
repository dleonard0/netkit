all: rcp

include ../MCONFIG
include ../MRULES

rcp: rcp.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rcp
	install -s -o root -m$(SUIDMODE) rcp $(BINDIR)
	install -m$(MANMODE) rcp.1 $(MANDIR)/man1

clean:
	rm -f *.o rcp

