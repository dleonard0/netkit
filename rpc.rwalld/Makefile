all: rwalld

include ../MCONFIG
include ../MRULES

CFLAGS += -I ../include

rwalld: rwalld.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -lbsd -o $@

install: rwalld
	install -s -m744 rwalld /usr/sbin/rpc.rwalld
	install -m644 rpc.rwalld.8 /usr/man/man8

clean:
	rm -f *.o rwalld

