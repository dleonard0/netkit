all:	lptest

lptest:	lptest.o

install: lptest
	install -s -m755 lptest /usr/bin
	install -m644 lptest.1 /usr/man/man1

clean:
	rm -f lptest *.o

