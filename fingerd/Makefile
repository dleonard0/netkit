all: fingerd

include ../MCONFIG
include ../MRULES

fingerd: fingerd.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

fingerd.o: pathnames.h

install: fingerd
	install -s -m$(DAEMONMODE) fingerd $(SBINDIR)/in.fingerd
	install -m$(MANMODE) fingerd.8 $(MANDIR)/man8/in.fingerd.8
	ln -sf in.fingerd.8 $(MANDIR)/man8/fingerd.8

clean:
	rm -f *.o fingerd

