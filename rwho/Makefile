all: rwho

include ../MCONFIG
include ../MRULES

CFLAGS += -I ../include

rwho: rwho.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rwho
	install -s rwho /usr/bin
	install -m644 rwho.1 /usr/man/man1/

clean:
	rm -f *.o rwho

