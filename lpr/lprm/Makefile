all:	lprm

lprm:	lprm.o

install: lprm
	install -s -m6555 lprm /usr/bin
	install -m644 lprm.1 /usr/man/man1

clean:
	rm -f lprm *.o

