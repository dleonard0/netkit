all: biff

include ../MCONFIG
include ../MRULES

biff: biff.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: biff
	install -s biff /usr/bin/
	install -m644 biff.1 /usr/man/man1/

clean:
	rm -f *.o biff

