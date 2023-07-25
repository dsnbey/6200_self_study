#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> // defines structure hostent

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
/*
 * Most codes are similar to the server code.
 * Refer to server.c to understand undocumented codes.
 */

/*
 * Socket functions for client:
 *
 * 1) socket(int domain, int type, int protocol) -> int sockfd
 *
 * 2) connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) -> int status
 *
 * 3) read(int newsockfd, char buffer[], int buffer_size) -> int bytes_read
 *
 * 4) write(int newsockfd, char buffer[], int buffer_size) -> int bytes_written
 *
 * 5) close(int sockfd)
 */
int main(int argc, char *argv[])
{

    // declarations
    int sockfd, portno, n;
    struct sockaddr_in serv_addr; // address of the server to connect
    struct hostent *server; // defines a host computer on the internet

    // Opening socket
    char buffer[256];
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

    /*
     * Establish a connection to server.
     *
     * Arguments:
     * 1) Socket file descriptor
     * 2) Address of the host to connect (including port number). Casted to sockaddr*
     * 3) Size of the address
     *
     * Return:
     * 0 -> Success
     * -1 -> Fail
     */
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    /*
     * It prompts the user to enter a message,
     * uses fgets to read the message from stdin,
     * writes the message to the socket,
     * reads the reply from the socket,
     * and displays this reply on the screen.
     */
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0)
        error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}