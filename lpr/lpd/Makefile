all:	lpd

lpd:	lpd.o lpdchar.o printjob.o recvjob.o

install: lpd
	install -s -m744 lpd /usr/sbin/lpd
	install -m644 lpd.8 /usr/man/man8

clean:
	rm -f lpd *.o

