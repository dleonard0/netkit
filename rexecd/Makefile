# -DRESTRICT_FTP if you want to deny all users in /etc/ftusers to use rexecd

ifdef HAVE_TCP_WRAPPER_LIBRARY
CFLAGS += -DTCP_WRAPPER
endif

CFLAGS += -DRESTRICT_FTP
# -DKERBEROS -DCRYPT
# -lutil -lkrb -ldes

rexecd: rexecd.o

install: rexecd
	install -s -m744 rexecd /usr/sbin/in.rexecd
	install -m644 rexecd.8 /usr/man/man8/in.rexecd.8

clean:
	rm -f *.o rexecd

