CFLAGS	+= -I../common_source
LDFLAGS	+= -N
LDLIBS += -lcommon -L ../common_source

SUBDIRS	= common_source filters lpc lpd lpq lpr lprm lptest pac

all:
	for i in $(SUBDIRS); do make -C $$i all; done

install:
	for i in $(SUBDIRS); do make -C $$i install; done
	install -m644 printcap.5 /usr/man/man5/

clean:
	for i in $(SUBDIRS); do make -C $$i clean; done

