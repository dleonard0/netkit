all: rexecd

include ../MCONFIG
include ../MRULES

ifeq ($(USE_GLIBC),1)
CFLAGS += -DGNU_LIBC -D_GNU_SOURCE
LIBS += -lcrypt
endif

#ifdef HAVE_TCP_WRAPPER_LIBRARY
#CFLAGS += -DTCP_WRAPPER
#endif

ifeq ($(USE_PAM),1)
CFLAGS += -DUSE_PAM
LIBS += -ldl -lpam
endif

ifeq ($(USE_SHADOW),1)
CFLAGS += -DUSE_SHADOW
LIBS += -lshadow
endif

# -DRESTRICT_FTP if you want to deny all users in /etc/ftpusers to use rexecd
CFLAGS += -DRESTRICT_FTP

rexecd: rexecd.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rexecd
	install -s -m$(DAEMONMODE) rexecd $(INSTALLROOT)$(SBINDIR)/in.rexecd
	install -m$(MANMODE) rexecd.8 $(INSTALLROOT)$(MANDIR)/man8/in.rexecd.8
	ln -sf in.rexecd.8 $(INSTALLROOT)$(MANDIR)/man8/rexecd.8

clean:
	rm -f *.o rexecd

