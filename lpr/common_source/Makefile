OBJS	= common.o displayq.o printcap.o rmjob.o startdaemon.o

all:	$(OBJS)
	@ar rcv libcommon.a $(OBJS)
	@ranlib libcommon.a

install: all

clean:
	rm -f *.o *.a

