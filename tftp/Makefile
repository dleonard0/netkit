CFLAGS += -Dsin=x_sin

tftp: tftp.o main.o tftpsubs.o

install: tftp
	install -s tftp /usr/bin
	install -m644 tftp.1 /usr/man/man1

clean:
	rm -f *.o tftp

