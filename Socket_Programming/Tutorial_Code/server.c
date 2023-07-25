/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


/*
 * NETWORKING TERMS:
 *
 * 1)Socket: A software endpoint that facilitates communication
 * between two network nodes, consisting of an IP address and a port number.
 *
 * 2)Port: A numeric identifier used to differentiate between
 * different services or processes running on a network node.
 *
 * 3)Address: An identifier that uniquely identifies a network node,
 * such as an IP address or MAC address.
 *
 * 4)Network Node: A device or a computer within a computer network
 * that can send, receive, or forward data.
 *
 * 5)IP Address: A unique numerical label assigned to a network device
 * that enables identification and communication within an IP-based network.
 *
 * 6)MAC Address: A unique identifier assigned to the network interface
 * of a device, used for communication within a local network.
 */



/*
 * Socket functions for server:
 *
 * 1) socket(int domain, int type, int protocol) -> int sockfd
 *
 * 2) bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) ->
 * int status: 0 for success, -1 for failure
 *
 * 3) listen(int sockfd, int backlog) -> int status: 0 for success, -1 for failure
 *
 * 4) accept(int sockfd, (struct sockaddr*) &addr, socklen_t &addrlen) -> int newsockfd
 * !Waits for connection() from client side
 *
 * 5) read(int newsockfd, char buffer[], int buffer_size) -> int bytes_read
 *
 * 6) write(int newsockfd, char buffer[], int buffer_size) -> int bytes_written
 *
 * 7) close(int sockfd)
 *
 */
int main(int argc, char *argv[])
{
    int sockfd, newsockfd; // file descriptor
    int portno;
    socklen_t clilen; // size of the address of client. Needed to accept syscalls
    char buffer[256]; // read into this buffer
    struct sockaddr_in serv_addr, cli_addr; // contains internet addresses
    int n; // return of read and write calls, contains number of chars read/written

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    /*
     * Create new socket.
     *
     * Arguments:
     * 1) domain: Address domain of socket. AF_UNIX-> Unix domain, AF_INET->Internet Domain
     * 2) type: SOCK_STREAM-> read as continuous stream as a pipe or file, SOCK_DGRAM-> Datagram, in chunks
     * 3) protocol: 0-> OS chooses the best protocol. TCP for streams, UDP for datagram
     *
     * Return: entry into file descriptor table. Used to reference to the socket. If fails, returns -1
     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
        error("ERROR opening socket");

    // sets all values in buffer to 0
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // takes port no as first arg
    portno = atoi(argv[1]);

    /*
     * Initialize fields of serv_addr
     * 1) sin_family: Code for Address family: Should be AF_INET
     * 2) sin_addr: contains only 1 field s_addr which contains IP of host. INADDR_ANY gets the machine's address.
     * 3) sin_port: contains port order. Needs to align with 'network byte order'
     */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /*
     * binds a socket to an address
     */
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    /*
     * Allows the process to listen on sockets
     *
     * Arguments:
     * 1)fd: socket file descriptor
     *
     * 2)n: size of backlog queue, i.e., the number of connections that can be
     * waiting while the process is handling a particular connection.
     * To learn the maximum size of backlog queue on linux machine,
     * run the following command: sysctl net.core.somaxconn
     */
    listen(sockfd,5);


    /*
     * accept() blocks the process until a client connects to the server.
     * Process unlocked when connection is established.
     *
     * Arguments:
     * 1) File descriptor of the socket of the server..
     * 2) Client address, cast to sockaddr *
     * 3) Size of the client address structure
     *
     * Return: File descriptor -> Communications should be done via this new descriptor
     */
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
    if (newsockfd < 0)
        error("ERROR on accept");


    /*
     * This portion is reached only after the connection is successfully established.
     *
     * Initialize the buffer and read from the socket. -> Read blocks until there is
     * something to read from the socket. ( after client executes write() )
     *
     */
    bzero(buffer,256);
    n = read(newsockfd,buffer,255); // Note that new descriptor is used!
    if (n < 0) error("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);


    // Both ends can write.
    n = write(newsockfd,"I got your message",18);
    if (n < 0) error("ERROR writing to socket");

    // Terminate the program.
    close(newsockfd);
    close(sockfd);
    return 0;
}