all: talkd

include ../MCONFIG
include ../MRULES

OBJS = talkd.o announce.o process.o table.o print.o

talkd: $(OBJS) 
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): proto.h

install: talkd
	install -s -m$(DAEMONMODE) talkd $(SBINDIR)/in.ntalkd
	ln -sf in.ntalkd $(SBINDIR)/in.talkd
	install -m$(MANMODE) talkd.8 $(MANDIR)/man8/in.ntalkd.8
	ln -sf in.ntalkd.8 $(MANDIR)/man8/in.talkd.8
	ln -sf ntalkd.8 $(MANDIR)/man8/in.ntalkd.8
	ln -sf talkd.8 $(MANDIR)/man8/in.talkd.8

clean:
	rm -f *.o talkd

