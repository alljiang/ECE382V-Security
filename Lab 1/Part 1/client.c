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

#define PORT 12000

int
main() {
	int rv;
	int socket_desc;
	struct sockaddr_in server_addr;
	char server_message[1024];
	char client_message[] = "Input lowercase sentence:";

	memset(server_message, '\0', sizeof(server_message));

	printf("client.c\n");

	// Create socket:
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc < 0) {
		printf("Unable to create socket\n");
		return -1;
	}

	// Set port and IP the same as server-side:
	server_addr.sin_family      = AF_INET;
	server_addr.sin_port        = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.2");

	// Send connection request to server:
	if (connect(socket_desc,
	            (struct sockaddr *) &server_addr,
	            sizeof(server_addr)) < 0) {
		printf("Unable to connect\n");
		return -1;
	}

	// Send the message to server:
	rv = send(socket_desc, client_message, strlen(client_message), 0);
    if (rv < 0) {
		printf("Unable to send message\n");
		return -1;
	}

	// Receive the server's response:
	rv = recv(socket_desc, server_message, sizeof(server_message), 0);
    if (rv < 0) {
		printf("Error while receiving server's msg\n");
		return -1;
	}

	printf("Server's response: %s\n", server_message);

	// Close the socket:
	close(socket_desc);

	return 0;
}
