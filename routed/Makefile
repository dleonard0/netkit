all: routed
include ../MCONFIG
include ../MRULES

CFLAGS += -I .

OBJECTS = af.o if.o inet.o input.o main.o output.o startup.o tables.o \
		timer.o trace.o

routed: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJECTS): defs.h pathnames.h trace.h table.h af.h interface.h

install: routed
	install -s -m744 routed /usr/sbin
	install -m644 routed.8 /usr/man/man8

clean:
	rm -f *.o routed

