all:	lpr

lpr:	lpr.o

install: lpr
	install -s -m6555 lpr /usr/bin
	install -m644 lpr.1 /usr/man/man1

clean:
	rm -f lpr *.o

