all: timed

include ../../MCONFIG
include ../../MRULES

LIBS += -lbsd

OBJS = timed.o acksend.o candidate.o correct.o master.o networkdelta.o \
	readmsg.o slave.o byteorder.o measure.o cksum.o

timed: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): globals.h pathnames.h timed-extern.h

install: timed
	install -s -m744 timed /usr/sbin/
	install -m644 timed.8 /usr/man/man8

clean:
	rm -f *.o timed

