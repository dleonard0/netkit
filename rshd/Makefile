# -DKERBEROS -DCRYPT
# -lutil -lkrb -ldes

rshd: rshd.o # des_rw.o

install: rshd
	install -s -m744 rshd /usr/sbin/in.rshd
	install -m644 rshd.8 /usr/man/man8/in.rshd.8

clean:
	rm -f *.o rshd

