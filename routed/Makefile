all: routed ripquery

include ../MCONFIG
include ../MRULES

OBJS=	af.o if.o input.o main.o output.o startup.o \
	tables.o timer.o trace.o inet.o

routed: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

ripquery:
	$(MAKE) -C query 

install: routed
	install -s -m$(DAEMONMODE) routed $(SBINDIR)
	install -m$(MANMODE) routed.8 $(MANDIR)/man8
	$(MAKE) -C query install

clean:
	rm -f *.o routed
	$(MAKE) -C query clean

$(OBJS): defs.h trace.h interface.h table.h af.h pathnames.h
