CFLAGS += -I ../include

rwall: rwall.o

install: rwall
	install -s -m755 rwall /usr/bin/
	install -m644 rwall.1 /usr/man/man1/

clean:
	rm -f *.o rwall

