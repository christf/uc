all: dir client

dir:
	mkdir -p bin/

binary: server client

server:
	gcc $(CFLAGS) -o bin/server server.c

client:
	gcc $(CFLAGS) -o bin/client client.c

install: client
	cp bin/client /usr/bin/uc
