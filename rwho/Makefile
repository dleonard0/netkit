CFLAGS += -I ../include

rwho: rwho.o

install: rwho
	install -s rwho /usr/bin
	install -m644 rwho.1 /usr/man/man1/

clean:
	rm -f *.o rwho

