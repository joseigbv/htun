# nativa
CC=gcc
CFLAGS=-Wall -O2 -s
LIBS=

# cross-compiling 
XCC=/opt/cross-compilers/sparc-sun-solaris2.9/bin/sparc-sun-solaris2.9-gcc
XCFLAGS=-Wall -O2
XLIBS=-lsocket -lnsl

DEPS=common.h config.h common.c

all: client server server-solsparc

client: $(DEPS) client.c 
	$(CC) $(CFLAGS) $(LIBS) common.c -o client client.c 


server: $(DEPS) server.c 
	$(CC) $(CFLAGS) $(LIBS) common.c -o server server.c 

server-solsparc: $(DEPS) server.c 
	$(XCC) $(XCFLAGS) $(XLIBS) common.c -o server-solsparc server.c 

clean: 
	rm client server server-solsparc

