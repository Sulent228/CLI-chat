#include <sys/epoll.h>  
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>                            
#include <stdlib.h>                                    
#include <string.h>  
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8000

int client_fd = -1;

void close_fd() {
    if (client_fd == -1) {return;}
    close(client_fd);
}

int main(void) {
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("error while creating client_fd");
        exit(EXIT_FAILURE);
    }
    atexit(close_fd);

    puts("created client_fd");

    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    struct sockaddr *server_addr_p = (struct sockaddr*)&server_addr;
    size_t server_addr_size = sizeof(server_addr);

    int connect_status = connect(client_fd, server_addr_p, server_addr_size);
    if (connect_status == -1) {
        perror("error while connecting to server");
        exit(EXIT_FAILURE);
    }

    puts("connected to server");

	char buffer[1024] = {0};
	while (1) {
		printf("waiting for input: ");
		char* input = fgets(buffer, sizeof(buffer)-1, stdin);

        if (input == NULL) {
            puts("error while reading input data");
            continue;
        }

        if (strcmp(buffer, "exit") == 0) break;

        size_t message_size = strlen(buffer)-1;
        printf("message size: %zu\n", message_size);

        if (message_size == 0) continue;

        send(client_fd, &message_size, sizeof(size_t), MSG_NOSIGNAL);
        ssize_t send_bytes = send(client_fd, buffer, message_size, MSG_NOSIGNAL);

        if (send_bytes == -1) {
            perror("error while sending message");
            continue;
        }
        puts("successfuly send data");
	}

    puts("exiting program");

    exit(EXIT_SUCCESS);
}
