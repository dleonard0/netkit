CFLAGS += -DMSG_EOR -Dosockaddr=sockaddr  # -DBAD_LINUX_HACK
LDLIBS += -lcurses

talk: ctl.o ctl_transact.o display.o get_addrs.o get_names.o \
	init_disp.o invite.o io.o look_up.o msgs.o talk.o

install: talk
	install -s talk /usr/bin/
	install -m644 talk.1 /usr/man/man1/

clean:
	rm -f *.o talk

