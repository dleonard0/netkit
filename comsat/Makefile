all: comsat

include ../MCONFIG
include ../MRULES

comsat: comsat.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: comsat
	install -s -m744 comsat /usr/sbin/in.comsat
	install -m644 comsat.8 /usr/man/man8/in.comsat.8

clean:
	rm -f *.o comsat

