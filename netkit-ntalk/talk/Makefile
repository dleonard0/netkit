all: talk

include ../MCONFIG
include ../MRULES

ifeq ($(USE_GLIBC),1)
CFLAGS += -DGNU_LIBC -D_GNU_SOURCE
endif

OBJS = ctl.o ctl_transact.o display.o get_addrs.o get_names.o \
	init_disp.o invite.o io.o look_up.o msgs.o talk.o

CFLAGS += -DMSG_EOR  # -DBAD_LINUX_HACK
LIBS += $(LIBCURSES)

talk: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): talk.h talk_ctl.h

install: talk
	install -s -m$(BINMODE) talk $(INSTALLROOT)$(BINDIR)
	install -m$(MANMODE) talk.1 $(INSTALLROOT)$(MANDIR)/man1/

clean:
	rm -f *.o talk

