OBJ =	docmd.o expand.o lookup.o main.o server.o gram.o

rdist:	$(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o rdist $(LDLIBS)

clean:
	rm -f *.o rdist

install: rdist
	install -s -m 4555 rdist /usr/bin/
	install -m 644 rdist.1 /usr/man/man1/

