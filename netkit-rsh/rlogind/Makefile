all: rlogind

include ../MCONFIG
include ../MRULES

OBJS = rlogind.o logwtmp.o

ifeq ($(USE_PAM),1)
OBJS += sockconv.o
CFLAGS += -DUSE_PAM
LIBS += -ldl -lpam -lpam_misc
endif

ifeq ($(USE_GLIBC),1)
CFLAGS += -DGNU_LIBC -D_GNU_SOURCE
LIBS += -lutil $(LIBTERMCAP)
else
LIBS += -lbsd
endif

rlogind: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): pathnames.h logwtmp.h

install: rlogind
	install -s -m$(DAEMONMODE) rlogind $(INSTALLROOT)$(SBINDIR)/in.rlogind
	install -m$(MANMODE) rlogind.8 $(INSTALLROOT)$(MANDIR)/man8/in.rlogind.8
	ln -sf in.rlogind.8 $(INSTALLROOT)$(MANDIR)/man8/rlogind.8

clean:
	rm -f *.o rlogind

