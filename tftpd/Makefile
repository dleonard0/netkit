all: tftpd

include ../MCONFIG
include ../MRULES

OBJS = tftpd.o ../tftp/tftpsubs.o
CFLAGS += -Dsin=x_sin

tftpd: tftpd.o ../tftp/tftpsubs.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: tftpd
	install -s -m744 tftpd /usr/sbin/in.tftpd
	install -m644 tftpd.8 /usr/man/man8/in.tftpd.8

clean:
	rm -f *.o tftpd

