# -DKERBEROS -DCRYPT
# -lkrb -ldes

rsh: rsh.o # krcmd.o kcmd.o des_rw.o

install: rsh
	install -s -m4755 rsh /usr/bin/rsh
	install -m644 rsh.1 /usr/man/man1

clean:
	rm -f *.o rsh

