all: ping

include ../MCONFIG
include ../MRULES

ping:	ping.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: ping
	install -s -o root -m4755 ping /bin
	install -m644 ping.8 /usr/man/man8

clean:
	rm -f *.o ping

