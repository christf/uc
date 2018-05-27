all: client

dir:
	mkdir -p bin/

binary: server client

server: dir
	gcc -Os -o bin/server server.c

client: dir
	gcc -Os -o bin/client client.c

install: client
	mkdir -p $(DESTDIR)/usr/bin
	install -m 0755 bin/client $(DESTDIR)/usr/bin/uc

