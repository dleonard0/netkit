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
	install -s ftp /usr/bin
	install -m644 ftp.1 /usr/man/man1
	ln -sf ftp /usr/bin/pftp

clean:
	rm -f *.o ftp

