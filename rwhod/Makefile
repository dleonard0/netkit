all: rwhod

include ../MCONFIG
include ../MRULES

CFLAGS += -I ../include

rwhod: rwhod.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -lbsd -o $@

install: rwhod
	install -s -m744 rwhod /usr/sbin/
	install -m644 rwhod.8 /usr/man/man8/

clean:
	rm -f *.o rwhod

