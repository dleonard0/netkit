LDFLAGS += -N

ping:	ping.o

install: ping
	install -s ping /sbin
	install -m644 ping.8 /usr/man/man8

clean:
	rm -f *.o ping

