CFLAGS = ${O} -Dinline=inline_flag
LDLIBS =

OBJECTS = rpc_clntout.o rpc_cout.o rpc_hout.o rpc_main.o rpc_parse.o \
  	  rpc_scan.o rpc_svcout.o rpc_tblout.o rpc_util.o rpc_sample.o

rpcgen: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o rpcgen

install: rpcgen
	install -s rpcgen /usr/bin

clean:
	rm -f *.o rpcgen

