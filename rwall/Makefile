all: rwall

include ../MCONFIG
include ../MRULES

CFLAGS += -I ../include

rwall: rwall.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rwall
	install -s -m755 rwall /usr/bin/
	install -m644 rwall.1 /usr/man/man1/

clean:
	rm -f *.o rwall

