#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

void error(const char *msg){
    perror(msg);
    exit(1);
}

void *receive_message(void *arg) {
    int newsockfd = *((int *)arg);
    char buffer[255];
    int n;
    while (1) {
        bzero(buffer, 255);
        n = read(newsockfd, buffer, 255);
        if (n < 0) {
            error("Error reading from socket");
        }
        printf("\nClient: %s", buffer);
    }
}

void *send_message(void *arg) {
    int newsockfd = *((int *)arg);
    char buffer[255];
    int n;
    while (1) {
        bzero(buffer, 255);
        fgets(buffer, 255, stdin);
        n = write(newsockfd, buffer, strlen(buffer));
        if (n < 0) {
            error("Error writing to socket");
        }
    }
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr,"Port number not provided, program terminated.\n");
        exit(1);
    }

    int sockfd, newsockfd, portno;
    struct sockaddr_in server_addr, client_addr;
    socklen_t clilen;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error opening socket");
    }

    bzero((char*)&server_addr, sizeof(server_addr));
    portno = atoi(argv[1]);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        error("Binding failed");
    }
    
    listen(sockfd, 5);
    clilen = sizeof(client_addr);

    newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &clilen);
    if (newsockfd < 0) {
        error("Error on accept");
    }

    pthread_t recv_thread, send_thread;
    if (pthread_create(&recv_thread, NULL, receive_message, (void *)&newsockfd) != 0) {
        error("Error creating receive thread");
    }
    if (pthread_create(&send_thread, NULL, send_message, (void *)&newsockfd) != 0) {
        error("Error creating send thread");
    }

    pthread_join(recv_thread, NULL);
    pthread_join(send_thread, NULL);
    close(newsockfd);
    close(sockfd);
    return 0;
}

