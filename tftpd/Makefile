all: tftpd

include ../MCONFIG
include ../MRULES

OBJS = tftpd.o tftpsubs.o

tftpd: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

tftpsubs.c: 
	ln -sf ../tftp/tftpsubs.c .
tftpsubs.h: 
	ln -sf ../tftp/tftpsubs.h .

$(OBJS): tftpsubs.h

install: tftpd
	install -s -m744 tftpd /usr/sbin/in.tftpd
	install -m644 tftpd.8 /usr/man/man8/in.tftpd.8

clean:
	rm -f *.o tftpd tftpsubs.c tftpsubs.h

