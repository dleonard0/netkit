timed: timed.o acksend.o candidate.o correct.o master.o networkdelta.o \
	readmsg.o slave.o byteorder.o measure.o cksum.o

install: timed
	install -s -m744 timed /usr/sbin/
	install -m644 timed.8 /usr/man/man8

clean:
	rm -f *.o timed

