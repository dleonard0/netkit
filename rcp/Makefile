all: rcp

include ../MCONFIG
include ../MRULES

rcp: rcp.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rcp
	install -s -o root -m4755 rcp /usr/bin
	install -m644 rcp.1 /usr/man/man1

clean:
	rm -f *.o rcp

