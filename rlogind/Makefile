all: rlogind

include ../MCONFIG
include ../MRULES

OBJS = rlogind.o logwtmp.o

ifeq ($(USE_PAM),1)
CFLAGS += -DUSE_PAM
LIBS += -ldl -lpam -lpam_misc
endif

rlogind: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -lbsd -o $@

rlogind.o logwtmp.o: pathnames.h

install: rlogind
	install -s -m744 rlogind /usr/sbin/in.rlogind
	install -m644 rlogind.8 /usr/man/man8/in.rlogind.8

clean:
	rm -f *.o rlogind

