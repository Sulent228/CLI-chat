#include <sys/epoll.h>  // Основной заголовок epoll
#include <unistd.h>     // Для close() (чтобы закрывать дескрипторы)
#include <fcntl.h>      // Для настройки флагов сокета (например, O_NONBLOCK)
#include <stdio.h>      // Для printf и perror
#include <stdlib.h>     // Для exit
#include <string.h>     // Для работы с памятью
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

    const char message[] = "Hello from client";

    ssize_t send_bytes = send(client_fd, message, sizeof(message), MSG_NOSIGNAL);
    if (send_bytes == -1) {
        perror("error while sending message to server");
        exit(EXIT_FAILURE);
    } else if (send_bytes == 0) {
        puts("could not send any data");
    } else {
        printf("Sended %zd bytes\n", send_bytes);
    }

    const char exit_message[] = "exit";

    send_bytes = send(client_fd, exit_message, sizeof(exit_message), MSG_NOSIGNAL);
    if (send_bytes == -1) {
        perror("error while sending exit_message to server");
        exit(EXIT_FAILURE);
    } else if (send_bytes == 0) {
        puts("could not send any data");
    } else {
        printf("Sended %zd bytes\n", send_bytes);
    }

    puts("exiting program");

    exit(EXIT_SUCCESS);
}
