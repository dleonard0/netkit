all: inetd

include ../MCONFIG
include ../MRULES

inetd: inetd.o daemon.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: inetd
	install -s -m744 inetd /usr/sbin/
	install -m644 inetd.8 /usr/man/man8/

clean:
	rm -f *.o inetd

