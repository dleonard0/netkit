all: talk

include ../MCONFIG
include ../MRULES

CFLAGS += -I../include

OBJS = ctl.o ctl_transact.o display.o get_addrs.o get_names.o \
	init_disp.o invite.o io.o look_up.o msgs.o talk.o

LIBS += $(LIBCURSES)

talk: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): talk.h ../include/prot_talkd.h ../version.h

install: talk
	install -s -m$(BINMODE) talk $(INSTALLROOT)$(BINDIR)
	install -m$(MANMODE) talk.1 $(INSTALLROOT)$(MANDIR)/man1/

clean:
	rm -f *.o talk

