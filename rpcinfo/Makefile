all: rpcinfo

include ../MCONFIG
include ../MRULES

rpcinfo: rpcinfo.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

install: rpcinfo
	install -s -m$(BINMODE) rpcinfo $(BINDIR)
	install -m$(MANMODE) rpcinfo.8 $(MANDIR)/man8

clean:
	rm -f *.o rpcinfo

