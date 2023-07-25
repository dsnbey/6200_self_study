// 3 hours so far-

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MESSAGE_SIZE 256
#define BUFFER_SIZE 1024
const char NOT_FOUND = '0';
const char OK = '1';
const char INTERNAL_ERROR = '2';
const char PUT_REPLACED = '3';
const char PUT_CREATED = '4';
const char DELETED = '5';

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void* get(const char* filename, int socket);
void* put(const char* filename, int socket);
void* delete(const char* filename, int socket);


int main(int argc, char *argv[]) {

    // BOILERPLATE SOCKET CODE
    // I HATE C. THIS SHOULD HAVE BEEN MUCH SIMPLER
    // I BETTER CODE MY OWN NETWORKING MODULES, OR IMPLEMENT MY OWN PROTOCOL
    // MAYBE MY OWN OS FOR MY SERVER
    // I HATE C BUT I HATE OTHER LANGUAGES MORE
    // GUESS I AM STUCK

    int sockfd, newsockfd;
    int portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)  error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_port = htons(portno);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;

    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd,5);

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
    if (newsockfd < 0)
        error("ERROR on accept");

    // SERVER PROTOCOL

    int option;
    char input[MESSAGE_SIZE]; // 0:MESSAGE

    while (1) {
        bzero(input, MESSAGE_SIZE);
        if (read(newsockfd, input, MESSAGE_SIZE) < 0) {
            error("Error reading from socket");
            continue;
        } else {
            option = input[0];

            const char* message = &input[2];

            switch (option) {
                case 0:
                    get(message, newsockfd);
                    break;
                case 1:
                    put(message, newsockfd);
                    break;
                case 2:
                    delete(message, newsockfd);
                    break;

            }

        }

    }
}

void* get(const char* filename, int socket) {
    FILE* file = fopen(filename, "r");

    if (!file) {
        send(socket, &NOT_FOUND, 1, 0);
    }
    else {
        char buffer[BUFFER_SIZE];
        size_t bytes_read;

        while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file))) {
            size_t total_bytes_send = 0;

            while (total_bytes_send < bytes_read) {
                size_t bytes_send = send(socket, buffer + total_bytes_send, bytes_read - total_bytes_send, 0);

                if (bytes_send == -1) {
                    perror("Send error");
                    return 0;
                }
                total_bytes_send += bytes_send;
            }
        }
    }

}
