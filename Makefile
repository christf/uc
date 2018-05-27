all: client

dir:
	mkdir -p bin/

binary: server client

server: dir
	gcc $(CFLAGS) -o bin/server server.c

client: dir
	gcc $(CFLAGS) -o bin/client client.c

install: client
	cp bin/client /usr/bin/uc
