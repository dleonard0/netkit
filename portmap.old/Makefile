LDFLAGS += -N

portmap: portmap.o

install: portmap
	install -s -m744 portmap /usr/sbin
#	install -m644 rpc /etc/rpc
	install -m644 portmap.8 /usr/man/man8

clean:
	rm -f *.o portmap

