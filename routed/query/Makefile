all: ripquery

include ../../MCONFIG
include ../../MRULES

OBJS=	query.o

ripquery: $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: ripquery
	install -o root -g root -m 0755 ripquery ${SBINDIR}
clean:
	rm -f $(OBJS) ripquery

