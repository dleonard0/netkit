ifdef USE_GNU_READLINE
CFLAGS += -D__USE_READLINE__ -Druserpass=x_ruserpass
LDLIBS += -lreadline -ltermcap
endif

CFLAGS += -Druserpass=x_ruserpass

ftp: cmds.o cmdtab.o domacro.o ftp.o glob.o main.o ruserpass.o

install: ftp
	install -s ftp /usr/bin
	install -m644 ftp.1 /usr/man/man1

clean:
	rm -f *.o ftp

