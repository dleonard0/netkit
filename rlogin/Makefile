all: rlogin

include ../MCONFIG
include ../MRULES

OBJS=rlogin.o

rlogin: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rlogin
	install -s -o root -m4755 rlogin /usr/bin
	install -m644 rlogin.1 /usr/man/man1

clean:
	rm -f *.o rlogin
