all: finger

include ../MCONFIG
include ../MRULES

finger: finger.o lprint.o net.o sprint.o util.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

finger.o lprint.o net.o sprint.o util.o: finger.h

install: finger
	install -s -m$(BINMODE) finger $(BINDIR)
	install -m$(MANMODE) finger.1 $(MANDIR)/man1

clean:
	rm -f *.o finger

