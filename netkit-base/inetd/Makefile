all: inetd

include ../MCONFIG
include ../MRULES

inetd: inetd.o servtab.o builtins.o sig.o daemon.o setproctitle.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

include depend.mk
depend:
	$(CC) $(CFLAGS) -MM *.c > depend.mk

install: inetd
	install -s -m$(DAEMONMODE) inetd $(INSTALLROOT)$(SBINDIR)
	install -m$(MANMODE) inetd.8 $(INSTALLROOT)$(MANDIR)/man8

clean:
	rm -f *.o inetd
