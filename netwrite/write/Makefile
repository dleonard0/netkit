include ../MCONFIG

TTYGRP=tty

all: write

write: write.c ../version.h
	$(CC) $(CFLAGS) write.c $(LDFLAGS) $(LIBS) -o write

install: write
	install -s -g $(TTYGRP) -m $(SGIDMODE) write $(INSTALLROOT)$(BINDIR)
	install -m$(MANMODE) write.1 $(INSTALLROOT)$(MANDIR)/man1

tidy:
	rm -f *~ core

clean: tidy
	rm -f write
