CFLAGS = ${O}
LDLIBS =

rpcinfo: rpcinfo.o

install: rpcinfo
	install -s -m744 rpcinfo /usr/sbin
	install -m644 rpcinfo.8 /usr/man/man8

clean:
	rm -f *.o rpcinfo

