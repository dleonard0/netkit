all: rwhod

include ../MCONFIG
include ../MRULES

CFLAGS += -I ../include
OBJS = rwhod.o daemon.o

rwhod: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): daemon.h

install: rwhod
	install -s -m744 rwhod /usr/sbin/
	install -m644 rwhod.8 /usr/man/man8/

clean:
	rm -f *.o rwhod

