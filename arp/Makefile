all: arp

include ../MCONFIG
include ../MRULES

arp: arp.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: arp
	install -s -m744 arp /usr/sbin
	install -m644 arp.8 /usr/man/man8

clean:
	rm -f *.o arp

