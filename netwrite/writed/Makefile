include ../MCONFIG

all: writed

writed: writed.c ../version.h
	$(CC) $(CFLAGS) -DBINDIR=\"$(BINDIR)\" writed.c $(LDFLAGS) $(LIBS) -o writed

install: writed
	install -s -m$(DAEMONMODE) writed $(INSTALLROOT)$(SBINDIR)/in.writed
	install -m$(MANMODE) writed.8 $(INSTALLROOT)$(MANDIR)/man8/in.writed.8
	ln -sf in.writed.8 $(INSTALLROOT)$(MANDIR)/man8/writed.8

tidy:
	rm -f *~ core

clean: tidy
	rm -f writed
