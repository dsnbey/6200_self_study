// 7 hours so far-

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MESSAGE_SIZE 256
#define BUFFER_SIZE 1024
const char NOT_FOUND = '0';
const char OK = '1';
const char INTERNAL_ERROR = '2';

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void* get(const char* filename, int socket);
void* put(const char* filename, int socket);
void* delete(const char* filename, int socket);


int main(int argc, char* argv[]){

    // declarations
    int sockfd, portno, n;
    struct sockaddr_in serv_addr; // address of the server to connect
    struct hostent *server; // defines a host computer on the internet

    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");


    // contains the name of a host on the Internet
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    /*
     * Set fields in serv_addr
     */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");


    while (1) {
        char buffer[BUFFER_SIZE];

        printf("Enter the option for the server (0-> get, 1-> put, 2-> delete):  ");
        scanf(" %c", &buffer[0]); // Note the space before %c to consume any leading whitespace

        printf("Enter the filename to perform the operation: ");
        scanf("%255s", &buffer[2]); // Start writing from the third element

        // Add the ':' character as the second element in the buffer
        buffer[1] = ':';

        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) {
            error("ERROR writing to socket");
            continue;
        }



        switch (buffer[0]) {
            case '0':
                get(&buffer[2], sockfd);
                break;
            case '1':
                put(&buffer[2], sockfd);
                break;
            case '2':
                delete(&buffer[2], sockfd);
                break;
            default:
                printf("PLease enter a valid option.\n");

        }

        bzero(buffer,256);

    }
}

void* get(const char* filename, int socket) {

     // Receive status from server
    char status[1];
    recv(socket, status, 1, 0);
    if (status[0] == NOT_FOUND) {
        printf("File requested not found\n");
        return 0;
    } else{
        printf("Request received successfully by the server.\n");
    }


    // Receive file size from server
    size_t file_size;
    if (recv(socket, &file_size, sizeof(file_size), 0) <= 0) {
        perror("Receive error");
        return 0;
    }

    if (file_size == 0) {
        perror("File not found");
        return 0;
    }

    // Open the file to write the received data
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file");
        return 0;
    }

    // Receive file data from server
    size_t total_bytes_received = 0;
    while (total_bytes_received < file_size) {
        char file_buffer[BUFFER_SIZE];
        ssize_t bytes_received = recv(socket, file_buffer, BUFFER_SIZE, 0);

        if (bytes_received <= 0) {
            perror("Receive error");
            break;
        }

        // Write received data to file
        fwrite(file_buffer, 1, bytes_received, file);
        total_bytes_received += bytes_received;
    }

    // Close the file
    fclose(file);
}

/*
 * In a production code, put should only send the changes, not the whole file.
 * However, this is no production, this code is for learning purposes.
 * Therefore, client will send the whole file and server will DELETE the file and RECREATE IT.
 * YES STUPIDITY, LESS GOOO BRRRR!!!
 */
void* put(const char* filename, int socket) {

    printf("PUT called");

    FILE* file = fopen(filename, "r");

    char s[1];
    if (!file) {
        printf("File not found in client.\n");
        s[0] = NOT_FOUND;
        send(socket, s, 1, 0);
        return 0;
    }
    s[0] = OK;
    send(socket, s, 1, 0);

    // Send the filesize
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    send(socket, &file_size, sizeof(file_size), 0);



    //  Receive the status code
    char status[1];
    recv(socket, status, 1, 0);
    if (status[0] == INTERNAL_ERROR) {
        perror("Internal error occurred in the server. Please try it another time.\n");
        return 0;
    } else{
        printf("Request received successfully by the server.\n");
    }


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
    printf("All data should be successfully received.\n");
    fclose(file);


}
void* delete(const char* filename, int socket) {
    char status[1];
    recv(socket, status, 1, 0);
    if (status[0] == INTERNAL_ERROR) {
        perror("Internal error occurred in the server. Please try it another time.\n");
    } else if (status[0] == NOT_FOUND){
        perror("File not found in the server.");
    }
    else{
        printf("Request received successfully by the server.\n");
    }
    return 0;

}

