all: tftp

include ../MCONFIG
include ../MRULES

OBJS = tftp.o main.o tftpsubs.o

tftp: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): tftpsubs.h

install: tftp
	install -s tftp /usr/bin
	install -m644 tftp.1 /usr/man/man1

clean:
	rm -f *.o tftp

