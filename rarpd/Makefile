all: rarpd

include ../MCONFIG
include ../MRULES

rarpd: rarpd.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rarpd
	install -s rarpd /usr/sbin
	install -m644 rarpd.8 /usr/man/man8

clean:
	rm -f *.o rarpd

