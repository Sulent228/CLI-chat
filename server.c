#include <sys/epoll.h> // Основной заголовок epoll
#include <unistd.h>     // Для close() (чтобы закрывать дескрипторы)
#include <fcntl.h>      // Для настройки флагов сокета (например, O_NONBLOCK)
#include <stdio.h>      // Для printf и perror
#include <stdlib.h>     // Для exit
#include <string.h>     // Для работы с памятью
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_ADDR "0.0.0.0"
#define SERVER_PORT 8000

#define LISTEN_BACKLOG 3

int server_fd = -1;

void close_fd() {
	if (server_fd == -1) {return;}
	close(server_fd);
}

int main(void) {
	// AF_INET - IPv4 ; SOCK_STREAM - TCP ; 0 - AUTO
	// __domain its about choosing IPv4 or IPv6
	// __type its about data transfering way
	// __protocol its about socket protocol
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		perror("error while creating server_fd");
		exit(EXIT_FAILURE);
	}
	atexit(close_fd);

	puts("Created server_fd");

	struct sockaddr_in server_addr = {0};

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	struct sockaddr *server_addr_p = (struct sockaddr*)&server_addr;
	size_t server_addr_size = sizeof(server_addr);

	int bind_status = bind(server_fd, server_addr_p, server_addr_size);
	if (bind_status == -1) {
		perror("error while binding server_fd");
		exit(EXIT_FAILURE);
	}

	puts("Binded server_fd");

	int listen_status = listen(server_fd, LISTEN_BACKLOG);
	if (listen_status == -1) {
		perror("error while turning server_fd to listen state");
		exit(EXIT_FAILURE);
	}

	puts("Turned server_fd to listen state");

	struct sockaddr_in client_addr = {0};

	struct sockaddr *client_addr_p = (struct sockaddr*)&client_addr;
	socklen_t client_addr_size = sizeof(client_addr);

	int client_fd = accept(server_fd, client_addr_p, &client_addr_size);
	if (client_fd == -1) {
		perror("error while accepting client");
		exit(EXIT_FAILURE);
	}

	puts("Accepted client");

	while (1) {
		int buffer[1024] = {0};

		ssize_t got_data_size = read(client_fd, &buffer, sizeof(buffer));
		if (got_data_size == -1) {
			perror("error while reading data from client_fd");
			exit(EXIT_FAILURE);
		} else if (got_data_size == 0) {
			puts("EOF");
		} else {
			buffer[got_data_size+1] = '\0';
			printf("got data:\n%s\n", (char*)buffer);
			if (strcmp("exit\0", (char*)buffer) == 0) {
				break;
			}
		}
	}

	puts("Exiting program");

	exit(EXIT_SUCCESS);
}
