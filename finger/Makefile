finger: finger.o lprint.o net.o sprint.o util.o

install: finger
	install -s finger /usr/bin
	install -m644 finger.1 /usr/man/man1

clean:
	rm -f *.o finger

