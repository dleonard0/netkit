# -DKERBEROS -DCRYPT
# -lutil -lkrb -ldes

rlogind: rlogind.o # des_rw.o

install: rlogind
	install -s -m744 rlogind /usr/sbin/in.rlogind
	install -m644 rlogind.8 /usr/man/man8/in.rlogind.8

clean:
	rm -f *.o rlogind

