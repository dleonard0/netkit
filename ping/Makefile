all: ping

include ../MCONFIG
include ../MRULES

ping:	ping.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: ping
	install -s -o root -m$(SUIDMODE) ping $(ROOTBINDIR)
	install -m$(MANMODE) ping.8 $(MANDIR)/man8

clean:
	rm -f *.o ping

