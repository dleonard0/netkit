CFLAGS += -DBSD=43
# -DKERBEROS -DCRYPT
# -lkrb -ldes

rlogin: rlogin.o # krcmd.o kcmd.o des_rw.o

install: rlogin
	install -s -m4755 rlogin /usr/bin
	install -m644 rlogin.1 /usr/man/man1

clean:
	rm -f *.o rlogin

