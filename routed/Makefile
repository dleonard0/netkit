CFLAGS += -I .
LDFLAGS	+= -N

OBJECTS = af.o if.o inet.o input.o main.o output.o startup.o tables.o \
		timer.o trace.o

routed: $(OBJECTS)
	$(CC) $(LDFLAGS) -o routed $(OBJECTS) $(LDLIBS)

install: routed
	install -s -m744 routed /usr/sbin
	install -m644 routed.8 /usr/man/man8

clean:
	rm -f *.o routed

