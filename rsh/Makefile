all: rsh

include ../MCONFIG
include ../MRULES

OBJS = rsh.o

rsh: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rsh
	install -s -o root -m$(SUIDMODE) rsh $(BINDIR)
	install -m$(MANMODE) rsh.1 $(MANDIR)/man1

clean:
	rm -f *.o rsh

