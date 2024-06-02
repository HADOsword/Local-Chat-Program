#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define BUFFER_SIZE 255

void error(const char *msg){
    perror(msg);
    exit(1);
}

void *receive_message(void *arg) {
    int sockfd = *((int *)arg);
    char buffer[BUFFER_SIZE];
    int n;
    while (1) {
        bzero(buffer, BUFFER_SIZE);
        n = read(sockfd, buffer, BUFFER_SIZE);
        if (n < 0) {
            error("Error reading from socket");
        }
        printf("\nServer: %s", buffer); // Print server messages
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr,"Usage: %s hostname port\n", argv[0]);
        exit(1);
    }

    int sockfd, portno, n;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char buffer[BUFFER_SIZE];

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error opening socket");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"Error, no such host\n");
        exit(1);
    }

    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Connection failed");
    }

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_message, (void *)&sockfd) != 0) {
        error("Error creating thread");
    }

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);

        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            error("Error writing to socket");
        }
    }

    close(sockfd);
    return 0;
}


