# You can do "make SUB=blah" to make only a few, or edit here, or both
# You can also run make directly in the subdirs you want.

SUB =   biff comsat finger fingerd ftp inetd \
	ping rcp rexecd rlogin rlogind rpcgen \
	rpc.rusersd rpc.rwalld rpc.bootparamd rpcinfo rsh \
	routed rshd rusers rwall rwho rwhod \
	talk talkd telnet telnetd tftp tftpd timed write

# These subdirs are not buildable - don't enable them:
# rup		(doesn't compile...)
#
# These programs are not supplied at all any more:
# arp		(use the arp in net-tools)
# ftpd		(use the 4.4BSD ftpd or wu_ftpd or ftpd-diku or...) 
# lpr		(use the PLP or LPRng package)
# portmap 	(use Wietse Venema's secure portmapper)
# rarpd 	(not useful with Linux)
# rdist		(use the rdist that's sold separately)
# slattach	(use slattach from net-tools)
# sliplogin	(use the sliplogin sold separately - check security notices)


%.build:
	$(MAKE) -C $(patsubst %.build, %, $@)

%.install:
	$(MAKE) -C $(patsubst %.install, %, $@) install

%.clean:
	$(MAKE) -C $(patsubst %.clean, %, $@) clean

all:     $(patsubst %, %.build, $(SUB))
install: $(patsubst %, %.install, $(SUB))
clean:   $(patsubst %, %.clean, $(SUB))
