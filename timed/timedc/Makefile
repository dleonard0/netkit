all: timedc

include ../../MCONFIG
include ../../MRULES

OBJS = timedc.o cmds.o cmdtab.o
LIBS += -L../lib -ltimed

timedc: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

$(OBJS): timedc.h timedc-extern.h

install: timedc
	install -s -m$(ROOTMODE) timedc $(SBINDIR)
	install -m$(MANMODE) timedc.8 $(MANDIR)/man8

clean:
	rm -f *.o timedc

