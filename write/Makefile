include ../MCONFIG

TTYGRP = tty
MODE = 2755

all: write writed

write: write.c
	$(CC) $(CFLAGS) write.c $(LDFLAGS) -o write

writed: writed.c
	$(CC) $(CFLAGS) -DBINDIR=\"$(BINDIR)\" writed.c $(LDFLAGS) -o writed

install: install-write install-writed

install-write: write
	install -s -g $(TTYGRP) -m $(MODE) write $(BINDIR)
	install -m$(MANMODE) write.1 $(MANDIR)/man1

install-writed: writed
	install -s -m$(DAEMONMODE) writed $(SBINDIR)/in.writed
	install -m$(MANMODE) writed.8 $(MANDIR)/man8/in.writed.8
	ln -sf in.writed.8 $(MANDIR)/man8/writed.8

tidy:
	rm -f *~ core

clean: tidy
	rm -f write writed
