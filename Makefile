# You can do "make SUB=blah" to make only a few, or edit here, or both
# You can also run make directly in the subdirs.

SUB =   biff comsat finger fingerd ftp inetd libtelnet \
	ping rcp rexecd rlogin rlogind rpcgen.new \
	rpc.rusersd rpc.rwalld rpcinfo rsh rshd rusers rwall rwho rwhod \
	slattach.new talk talkd telnet telnetd tftp tftpd timed

# These subdirs are not built:
# arp		(use the arp in net-tools)
# ftpd		(use the 4.4BSD ftpd or wu_ftpd or ftpd-diku or...) 
# lpr		(use the plp package)
# rarpd 	(doesn't compile...)
# routed	(deprecated due to being totally broken beyond hope; use gated)
# rup		(doesn't compile...)
# rpcgen	(obsolete)
# slattach	(obsolete)
#
# These programs are not supplied at all any more:
# portmap 	(use Wietse Venema's secure portmapper)
# rdist		(use the rdist that's sold separately)
# sliplogin	(use the sliplogin sold separately - check security notices)


all:
	for i in $(SUB); do make -C  $$i; done

install:
	for i in $(SUB); do make -C  $$i install; done

clean:
	for i in $(SUB); do make -C  $$i clean; done

