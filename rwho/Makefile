all: rwho

include ../MCONFIG
include ../MRULES

CFLAGS += -I ../include

rwho: rwho.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rwho
	install -s -m$(BINMODE) rwho $(BINDIR)
	install -m$(MANMODE) rwho.1 $(MANDIR)/man1/

clean:
	rm -f *.o rwho

