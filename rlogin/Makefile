all: rlogin

include ../MCONFIG
include ../MRULES

OBJS=rlogin.o

rlogin: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rlogin
	install -s -o root -m$(SUIDMODE) rlogin $(BINDIR)
	install -m$(MANMODE) rlogin.1 $(MANDIR)/man1

clean:
	rm -f *.o rlogin
