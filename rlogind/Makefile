all: rlogind

include ../MCONFIG
include ../MRULES

OBJS = rlogind.o logwtmp.o

ifeq ($(USE_PAM),1)
CFLAGS += -DUSE_PAM
LIBS += -ldl -lpam -lpam_misc
endif

ifeq ($(USE_GLIBC),1)
LIBS += -lutil $(LIBTERMCAP)
endif

rlogind: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -lbsd -o $@

$(OBJS): pathnames.h logwtmp.h

install: rlogind
	install -s -m$(DAEMONMODE) rlogind $(SBINDIR)/in.rlogind
	install -m$(MANMODE) rlogind.8 $(MANDIR)/man8/in.rlogind.8
	ln -sf in.rlogind.8 $(MANDIR)/man8/rlogind.8

clean:
	rm -f *.o rlogind

