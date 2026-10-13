#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_ADDR "0.0.0.0"
#define SERVER_PORT 8000

#define LISTEN_BACKLOG 3

struct socket_s {
	int fd;
	struct sockaddr_in* addr;
	socklen_t* socklen;
	size_t needtoread;
} socket_s;

struct epoll_event events[10] = {0};
int epfd = -1;

void socket_memory_free(struct socket_s* socket) {
	if (socket == NULL) return;
	if (socket->addr != NULL) free(socket->addr);
	if (socket->socklen != NULL) free(socket->socklen);
	free(socket);
}

void server_event_handler() {
	puts("Received event from server_fd");
	struct socket_s *new_client = calloc(1, sizeof(struct socket_s));

	new_client->addr = calloc(1, sizeof(struct sockaddr));
	new_client->socklen = calloc(1, sizeof(socklen_t));
	new_client->fd = accept(server_fd, new_client->addr, new_client->socklen);

	printf("Wrote %d to clients fd\n", new_client->fd);

	if (new_client->fd == -1) {
		perror("error while accepting client");
		socket_memory_free(new_client);
	}

	puts("Created client structure");

	struct epoll_event *client_fd_epoll_event = calloc(1, sizeof(struct epoll_event));
	client_fd_epoll_event->events = EPOLLIN;
	client_fd_epoll_event->data.ptr = new_client;

	int epoll_ctl_status = epoll_ctl(epfd, EPOLL_CTL_ADD, new_client->fd, client_fd_epoll_event);
	if (epoll_ctl_status == -1) {
		perror("error while registering client to epoll");
		socket_memory_free(new_client);
	}

	puts("Client registered");
}

void client_event_handler(struct socket_s *client_s) {
	puts("Received event from client_fd");

	int client_fd = client_s->fd;

	int buffer[1024] = {0};

	if (client_s->needtoread == 0) {
		read(client_fd, &client_s->needtoread, sizeof(size_t));
		printf("need to read: %zd\n", client_s->needtoread);
	}

	int got_data_size = read(client_fd, &buffer, client_s->needtoread);
	if (got_data_size == -1) { // error
		perror("error while reading data from client_fd");
	} else if (got_data_size == 0) { // client disconnected
		puts("Client disconnected");
		int epoll_socket_del_status = epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, NULL);
		if (epoll_socket_del_status == -1) {
			perror("error while deleting client from epoll");
			return;
		}
		close(client_fd);
		socket_memory_free(client_s);
	} else { // getting data
		buffer[got_data_size+1] = '\0';
		printf("got data:\n%s\n", (char*)buffer);
		client_s->needtoread = 0;
	}
}

struct socket_s* server_s_initialization() {
	struct socket_s *server_s = malloc(sizeof(struct socket_s));

	server_s->fd = -1;
	server_s->addr = malloc(sizeof(struct sockaddr_in));
	server_s->socklen = malloc(sizeof(socklen_t));

	server_s->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_s->fd == -1) {
		perror("error while creating server_fd");
		return NULL;
	}

	puts("Created server_fd");

	server_s->addr->sin_family = AF_INET;
	server_s->addr->sin_port = htons(SERVER_PORT);
	server_s->addr->sin_addr.s_addr = inet_addr(SERVER_ADDR);

	int bind_status = bind(server_s->fd, (struct sockaddr*)server_s->addr, *server_s->socklen);
	if (bind_status == -1) {
		perror("error while binding server_fd");
		return NULL;
	}

	puts("Binded server_fd");
	return server_s;
}

int main(void) {
	// AF_INET - IPv4 ; SOCK_STREAM - TCP ; 0 - AUTO
	// __domain its about choosing IPv4 or IPv6
	// __type its about data transfering way
	// __protocol its about socket protocol
	struct socket_s *server_s = server_s_initialization();
	if (server_s == NULL) {
		perror("could not initialize server");
		exit(EXIT_FAILURE);
	}

	int listen_status = listen(server_s->fd, LISTEN_BACKLOG);
	if (listen_status == -1) {
		perror("error while turning server to listen state");
		exit(EXIT_FAILURE);
	}

	puts("Turned server to listen state");

	epfd = epoll_create1(0);
	if (epfd == -1) {
		perror("error while creating epoll file descriptor");
		exit(EXIT_FAILURE);
	}

	puts("Created epoll fd");

	struct epoll_event server_fd_epoll_event = {0};

	struct socket_s server_s = {0};
	server_s.fd = server_fd;
	server_s.addr = NULL;
	server_s.socklen = NULL;

	printf("Wrote %d in server_s fd\n", server_s.fd);

	server_fd_epoll_event.events = EPOLLIN;
	server_fd_epoll_event.data.ptr = &server_s;

	printf("in server_fd %d\n", server_fd);

	int epoll_ctl_status = epoll_ctl(epfd, EPOLL_CTL_ADD, server_s.fd, &server_fd_epoll_event);
	if (epoll_ctl_status == -1) {
		perror("error while registering server_fd to epoll");
		exit(EXIT_FAILURE);
	}

	bool mustclose = 0;
	while (!mustclose) {
		int event_count =  epoll_wait(epfd, events, 10, 1000);
		if (event_count > 0) printf("events caused: %d\n", event_count);
		for (int i = 0; i < event_count; i++) {
			struct epoll_event event = events[i]; // getting current caused event
			uint32_t event_events = event.events; // getting events
			struct socket_s *event_data_pointer = event.data.ptr; // pointer to socket_s
			int event_fd = event_data_pointer->fd; // file descriptor of socket
			printf("event %d caused %d from file descriptor %d\n", i, event_events, event_fd);
			// sleep(1);
			if (event_events == EPOLLIN) {
				if (event_fd == server_fd) {
					server_event_handler();
				} else {
					client_event_handler(event_data_pointer);
				}
			}
		}
	}

	puts("Exiting program");

	exit(EXIT_SUCCESS);
}
