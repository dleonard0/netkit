all: comsat

include ../MCONFIG

%.o: %.c
	$(CC) $(CFLAGS) $< -c

comsat.o: ../version.h

comsat: comsat.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: comsat
	install -s -m$(DAEMONMODE) comsat $(INSTALLROOT)$(SBINDIR)/in.comsat
	install -m$(MANMODE) comsat.8 $(INSTALLROOT)$(MANDIR)/man8/in.comsat.8
	ln -sf in.comsat.8 $(MANDIR)/man8/comsat.8

clean:
	rm -f *.o comsat

