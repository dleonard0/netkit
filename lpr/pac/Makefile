all:	pac

pac:	pac.o

install: pac
	install -s -m744 pac /usr/sbin
	install -m644 pac.8 /usr/man/man8

clean:
	rm -f pac *.o

