# You can do "make SUB=blah" to make only a few, or edit here, or both
# You can also run make directly in the subdirs you want.

include MCONFIG

ifneq ($(USE_GLIBC),1)

SUB = rpcgen rpcinfo

%.build:
	(cd $(patsubst %.build, %, $@) && $(MAKE))

%.install:
	(cd $(patsubst %.install, %, $@) && $(MAKE) install)

%.clean:
	(cd $(patsubst %.clean, %, $@) && $(MAKE) clean)

all:     $(patsubst %, %.build, $(SUB))
install: $(patsubst %, %.install, $(SUB))
clean:   $(patsubst %, %.clean, $(SUB))

else

# on glibc systems this whole package should not be used

all install clean:
	@echo 'glibc system; skipping netkit-rpc'

endif

distclean: clean
	rm -f MCONFIG

