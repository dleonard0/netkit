all: timedc

include ../../MCONFIG
include ../../MRULES

timedc: timedc.o cmds.o cmdtab.o byteorder.o measure.o cksum.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

byteorder.o: ../timed/byteorder.c
	$(CC) $(CFLAGS) -c ../timed/byteorder.c -o byteorder.o
measure.o: ../timed/measure.c
	$(CC) $(CFLAGS) -c ../timed/measure.c -o measure.o
cksum.o: ../timed/cksum.c
	$(CC) $(CFLAGS) -c ../timed/cksum.c -o cksum.o

install: timedc
	install -s -m4744 timedc /usr/sbin/
	install -m644 timedc.8 /usr/man/man8

clean:
	rm -f *.o timedc

