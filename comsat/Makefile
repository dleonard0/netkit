all: comsat

include ../MCONFIG
include ../MRULES

comsat: comsat.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: comsat
	install -s -m$(DAEMONMODE) comsat $(SBINDIR)/in.comsat
	install -m$(MANMODE) comsat.8 $(MANDIR)/man8/in.comsat.8
	ln -sf in.comsat.8 $(MANDIR)/man8/comsat.8

clean:
	rm -f *.o comsat

