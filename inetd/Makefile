all: inetd

include ../MCONFIG
include ../MRULES

inetd: inetd.o daemon.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

inetd.o daemon.o: pathnames.h inetd.h

install: inetd
	install -s -m$(DAEMONMODE) inetd $(SBINDIR)
	install -m$(MANMODE) inetd.8 $(MANDIR)/man8

clean:
	rm -f *.o inetd

