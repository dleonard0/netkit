all:	lpc

lpc:	lpc.o cmds.o cmdtab.o

install: lpc
	install -s -g lp -m2555 lpc /usr/sbin/lpc
	install -m644 lpc.8 /usr/man/man8

clean:
	rm -f lpc *.o

