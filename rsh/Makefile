all: rsh

include ../MCONFIG
include ../MRULES

OBJS = rsh.o

rsh: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rsh
	install -s -o root -m4755 rsh /usr/bin/rsh
	install -m644 rsh.1 /usr/man/man1

clean:
	rm -f *.o rsh

