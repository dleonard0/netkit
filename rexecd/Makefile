all: rexecd

include ../MCONFIG
include ../MRULES

#ifdef HAVE_TCP_WRAPPER_LIBRARY
#CFLAGS += -DTCP_WRAPPER
#endif

ifeq ($(USE_PAM),1)
CFLAGS += -DUSE_PAM
LDFLAGS += -ldl -lpam
endif

# -DRESTRICT_FTP if you want to deny all users in /etc/ftpusers to use rexecd
CFLAGS += -DRESTRICT_FTP

rexecd: rexecd.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rexecd
	install -s -m744 rexecd /usr/sbin/in.rexecd
	install -m644 rexecd.8 /usr/man/man8/in.rexecd.8

clean:
	rm -f *.o rexecd

