all: client

dir:
	mkdir -p bin/

binary: server client

server: dir
	gcc -Os -o bin/server server.c

client: dir
	gcc -Os -o bin/client client.c

install: client
	cp bin/client /usr/bin/uc
