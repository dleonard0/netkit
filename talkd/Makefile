CFLAGS += -Dosockaddr=sockaddr

talkd: talkd.o announce.o process.o table.o print.o

install: talkd
	install -s -m744 talkd /usr/sbin/in.talkd
	install -m644 talkd.8 /usr/man/man8/in.talkd.8

clean:
	rm -f *.o talkd

