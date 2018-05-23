all: dir client

dir:
	mkdir -p bin/

binary: server client

server:
	gcc -Os -o bin/server server.c

client:
	gcc -Os -Wall -o bin/client client.c
