all: 
	@echo "Please don't use this program."

include ../MCONFIG
include ../MRULES

# comment this out, if you don't have shadow passwords on your system
# This will affect the compilation of the subdirs: ftpd
HAVE_SHADOW_PASSWORDS=true

ifdef HAVE_SHADOW_PASSWORDS
CFLAGS += -DSHADOW_PWD
LIBS += -lshadow
endif

CFLAGS += -include ftphack.h -DSETPROCTITLE -I../ftp
YACC = bison -y

ftpd: ftpd.o ftpcmd.o glob.o logwtmp.o popen.o vers.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

glob.o: ../ftp/glob.c
	$(CC) $(CFLAGS) -c ../ftp/glob.c -o glob.o

install: ftpd
	install -s -m744 ftpd /usr/sbin/in.ftpd
	install -m644 ftpd.8 /usr/man/man8/in.ftpd.8

clean:
	rm -f *.o ftpd

