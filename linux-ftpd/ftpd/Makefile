include ../MCONFIG

CFLAGS+=-I../support
LIBS+=-L../support -lsupport


OBJS=ftpd.o ftpcmd.o logutmp.o logwtmp.o popen.o

all: ftpd

%.o: %.c
	$(CC) $(CFLAGS) -DHASSETPROCTITLE $< -c

ftpcmd.c: %.c: %.y
	$(YACC) $<
	-mv -f y.tab.c $@

ftpd: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: ftpd
	install -s -o root -m$(DAEMONMODE) ftpd $(SBINDIR)/in.ftpd
	install -m$(MANMODE) ftpd.8 $(MANDIR)/man8/in.ftpd.8
	ln -sf in.ftpd.8 $(MANDIR)/man8/ftpd.8

clean:
	rm -f *.o ftpcmd.c y.tab.h ftpd


ftpd.o: pathnames.h
$(OBJS): extern.h ../support/daemon.h ../support/setproctitle.h
