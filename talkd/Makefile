all: talkd

include ../MCONFIG
include ../MRULES

CFLAGS += -Dosockaddr=sockaddr
OBJS = talkd.o announce.o process.o table.o print.o

talkd: $(OBJS) 
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): proto.h

install: talkd
	install -s -m744 talkd /usr/sbin/in.ntalkd
	ln -sf in.ntalkd /usr/sbin/in.talkd
	install -m644 talkd.8 /usr/man/man8/in.ntalkd.8
	ln -sf in.ntalkd.8 /usr/man/man8/in.talkd.8

clean:
	rm -f *.o talkd

