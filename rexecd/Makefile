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

ifeq ($(USE_SHADOW),1)
CFLAGS += -DUSE_SHADOW
LDFLAGS += -lshadow
endif

# -DRESTRICT_FTP if you want to deny all users in /etc/ftpusers to use rexecd
CFLAGS += -DRESTRICT_FTP -DSHADOW_PWD

rexecd: rexecd.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rexecd
	install -s -m$(DAEMONMODE) rexecd $(SBINDIR)/in.rexecd
	install -m$(MANMODE) rexecd.8 $(MANDIR)/man8/in.rexecd.8
	ln -sf in.rexecd.8 $(MANDIR)/man8/rexecd.8

clean:
	rm -f *.o rexecd

