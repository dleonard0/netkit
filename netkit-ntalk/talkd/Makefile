all: talkd

include ../MCONFIG
include ../MRULES

ifeq ($(USE_GLIBC),1)
CFLAGS += -DGNU_LIBC -D_GNU_SOURCE
endif

OBJS = talkd.o announce.o process.o table.o print.o

talkd: $(OBJS) 
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): proto.h

install: talkd
	install -s -m$(DAEMONMODE) talkd $(INSTALLROOT)$(SBINDIR)/in.ntalkd
	ln -sf in.ntalkd $(INSTALLROOT)$(SBINDIR)/in.talkd
	install -m$(MANMODE) talkd.8 $(INSTALLROOT)$(MANDIR)/man8/in.ntalkd.8
	ln -sf in.ntalkd.8 $(INSTALLROOT)$(MANDIR)/man8/in.talkd.8
	ln -sf in.ntalkd.8 $(INSTALLROOT)$(MANDIR)/man8/ntalkd.8
	ln -sf in.talkd.8 $(INSTALLROOT)$(MANDIR)/man8/talkd.8

clean:
	rm -f *.o talkd

