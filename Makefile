# You can do "make SUB=blah" to make only a few, or edit here, or both
# You can also run make directly in the subdirs you want.

SUB = netkit-base netkit-rpc \
      netkit-ntalk bsd-finger linux-ftpd netkit-ftp netwrite \
      netkit-bootparamd netkit-tftp \
      biff+comsat netkit-rusers netkit-rwho netkit-rwall \
      netkit-routed netkit-rsh netkit-telnet netkit-timed


%.build:
	(cd $(patsubst %.build, %, $@) && $(MAKE))

%.depend:
	(cd $(patsubst %.depend, %, $@) && $(MAKE) depend)

%.install:
	(cd $(patsubst %.install, %, $@) && $(MAKE) install)

%.clean:
	(cd $(patsubst %.clean, %, $@) && $(MAKE) clean)

%.distclean:
	(cd $(patsubst %.distclean, %, $@) && $(MAKE) distclean)

all:     $(patsubst %, %.build, $(SUB))
depend:   $(patsubst %, %.depend, $(SUB))
install: $(patsubst %, %.install, $(SUB))
clean:   $(patsubst %, %.clean, $(SUB))
distclean: $(patsubst %, %.distclean, $(SUB)) my-distclean

my-distclean:
	-rm -f configure.defs
