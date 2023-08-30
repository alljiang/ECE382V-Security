#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 12000


int
main() {
	int rv;
	int client_size;
    int socket_desc, client_sock;
	char server_message[1024], client_message[1024];

    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

	printf("server.c\n");

	// Create a socket
	struct sockaddr_in server_address, client_address;

	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc < 0) {
		printf("Error: Could not create socket\n");
		return 1;
	}

	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(SERVER_PORT);
	server_address.sin_addr.s_addr = inet_addr("127.0.0.2");

	rv = bind(socket_desc,
	          (struct sockaddr *) &server_address,
	          sizeof(server_address));
	if (rv < 0) {
		printf("Error: Could not bind socket\n");
		return 1;
	}

	if (listen(socket_desc, 1) < 0) {
		printf("Error: Could not listen on socket\n");
		return 1;
	}

    printf("The server is ready to receive\n");

	while (1) {
		client_size = sizeof(client_address);
		client_sock = accept(socket_desc,
		                     (struct sockaddr *) &client_address,
		                     (socklen_t *) &client_size);
		if (client_sock < 0) {
			printf("Error: Could not accept connection\n");
			return 1;
		}

		printf("%s\n", inet_ntoa(client_address.sin_addr));

		rv = recv(client_sock, client_message, sizeof(client_message), 0);
		if (rv < 0) {
			printf("Error: Could not receive message\n");
			return 1;
		}

		int length = rv;
		for (int i = 0; i < length; i++) {
			server_message[i] = toupper(client_message[i]);
		}

		rv = send(client_sock, server_message, length, 0);
		if (rv < 0) {
			printf("Error: Could not send message\n");
			return 1;
		}

		close(client_sock);
	}

	return 0;
}
