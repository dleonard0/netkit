all: ftp

include ../MCONFIG
include ../MRULES

ifeq ($(USE_GNU_READLINE),1)
CFLAGS += -D__USE_READLINE__
LIBS += -lreadline $(LIBTERMCAP)
endif

ftp: cmds.o cmdtab.o domacro.o ftp.o glob.o main.o ruserpass.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

domacro.o ftp.o glob.o main.o ruserpass.o: ftp_var.h pathnames.h
cmds.o cmdtab.o: ftp_var.h pathnames.h cmds.h
cmds.o glob.o: glob.h

install: ftp
	install -s -m$(BINMODE) ftp $(BINDIR)
	ln -sf ftp $(BINDIR)/pftp
	install -m$(MANMODE) ftp.1 $(MANDIR)/man1
	ln -sf ftp.1 $(MANDIR)/man1/pftp.1

clean:
	rm -f *.o ftp

