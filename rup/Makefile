all: rup

include ../MCONFIG
include ../MRULES

rup: rup.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rup
	install -s -m744 rup /usr/bin/
	install -m644 rup.1 /usr/man/man1/

clean:
	rm -f *.o rup

