DIRS=support ftpd
TARGETS=all clean install

#
# This piece of somewhat dubious gmake hackery applies each of the listed
# targets in each of the listed dirs. The first listed target is the
# default.

$(TARGETS): %: $(patsubst %, %.%, $(DIRS))

$(foreach TGT, $(TARGETS), $(patsubst %, %.$(TGT), $(DIRS))):
	$(MAKE) -C $(subst ., , $@)

