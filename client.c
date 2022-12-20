/*
   Copyright (c) 2018, Christof Schulze <christof.schulze@gmx.net>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   */

#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define MAXEVENTS 16

void add_fd ( int efd, int fd, uint32_t events )
{
	struct epoll_event event = {};
	event.data.fd = fd;
	event.events = events;

	int s = epoll_ctl ( efd, EPOLL_CTL_ADD, fd, &event );
	if ( s == -1 ) {
		perror ( "epoll_ctl (ADD):" );
		exit(-1);
	}
}

int main(int argc, char **argv) {
	int fd;
	char buff[BUFFER_SIZE] = {};
	int ok = 1;

	if (argc != 2) {
		printf("send the first line of data received on stdin to the unix socket\n and display the response\n\n");
		printf("synopsis: uc <unix socket>\n");
		return 1;
	}

	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		ok = 0;
	}

	if (ok) {
		struct sockaddr_un addr = {};
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, argv[1]);
		if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
			perror("connect");
			ok = 0;
		}
	}

	if (ok) {
		size_t inputlength=0;
		char c;
		memset(buff, 0, BUFFER_SIZE);
		while (EOF != (c = fgetc(stdin)) && inputlength < BUFFER_SIZE) {
			buff[inputlength] = c;
			inputlength++;
		}
		if (send(fd, buff, inputlength, 0) == -1) {
			perror("send");
			ok = 0;
		}

		if (shutdown(fd, SHUT_WR) ) {
			perror("shutdown");
		}
	}

	struct epoll_event *events = NULL;
	if (ok) {
		size_t output_len;
		events = malloc(MAXEVENTS * sizeof(struct epoll_event));
		if (!events) {
			perror("malloc");
			ok = 0;
			goto cleanup;
		}

		int efd = epoll_create1 ( 0 );
		if ( efd == -1 ) {
			perror ( "epoll_create" );
			ok = 0;
			goto cleanup;
		}
		add_fd ( efd, fd, EPOLLIN );

		while (1) {
			int n = epoll_wait ( efd, events, MAXEVENTS, -1 );

			for ( int i = 0; i < n; i++ ) {
				if (events[i].data.fd == fd ) {
					output_len = recv(fd, buff, BUFFER_SIZE -1, MSG_DONTWAIT);
					if ( ( output_len == -1 ) && ( errno != EAGAIN ) ) {
						perror ( "read error" );
						ok = 0;
						goto cleanup;
					} else if ( output_len == -1 ) {
						break; // errno must be EAGAIN - we have read all data that is currently available. FD is still open.
					} else if ( output_len == 0 ) {
						goto cleanup; // received EOF, closing connection
					}
					printf ("%s", buff);
					fflush(stdout);
					memset(buff, 0, output_len);
				}
			}
		}
	}

cleanup:
	free(events);
	if (fd >= 0) {
		close(fd);
	}
	return ! ok;
}
