all: slattach

include ../MCONFIG
include ../MRULES

slattach: slattach.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: slattach
	install -s -m744 slattach /usr/sbin/bsdslattach
	install -m644 slattach.8 /usr/man/man8/bsdslattach.8

clean:
	rm -f *.o slattach

