all: rpcinfo

include ../MCONFIG
include ../MRULES

rpcinfo: rpcinfo.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rpcinfo
	install -s -m744 rpcinfo /usr/bin
	install -m644 rpcinfo.8 /usr/man/man8

clean:
	rm -f *.o rpcinfo

