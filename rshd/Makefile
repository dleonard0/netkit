all: rshd

include ../MCONFIG
include ../MRULES

OBJS = rshd.o

ifeq ($(USE_PAM),1)
CFLAGS += -DUSE_PAM
LIBS += -ldl -lpam -lpam_misc
endif

rshd: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -lbsd -o $@

install: rshd
	install -s -m744 rshd /usr/sbin/in.rshd
	install -m644 rshd.8 /usr/man/man8/in.rshd.8

clean:
	rm -f *.o rshd

