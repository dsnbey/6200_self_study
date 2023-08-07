// 14 hours so far-

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <pthread.h>
#include "hashtable/hashtable.h" // allah belanızı versin

#define MESSAGE_SIZE 256
#define BUFFER_SIZE 1024
#define THREAD_COUNT 100
const char NOT_FOUND = '0';
const char OK = '1';
const char INTERNAL_ERROR = '2';
HashTable table;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

typedef struct operation_args {
    sem_t* write_lock, *read_lock, *no_readers, *no_writers;
    int newsockfd;
    const char* filename;
}operation_args;

void* operator_thread(void* arg_v);
void* get(void* arg_v);
void* put(void* arg_v);
void* delete(void* arg_v);
operation_args* get_op_args(int sockfd, const char* filename);

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

    // initialize table
    ht_setup(&table, sizeof(int), sizeof(operation_args), 100);
    ht_reserve(&table, 100);

    pthread_t tid[THREAD_COUNT];
    int newsockfds[THREAD_COUNT];
    int i = 0;


    while (1) {

        printf("Accept will be called with i = %d", i);
        newsockfds[i] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        printf("Accept called");
        if (newsockfds[i] < 0)
            error("ERROR on accept");

        pthread_create(&tid[i], NULL, operator_thread, &newsockfds[i]);
        i++;
        if (i == THREAD_COUNT + 1) {
            // Todo: Needs better error handling. This is just a patch.
            error("Max count reached.");
        }

    }


}

void* operator_thread(void* arg_v){

    int *newsockfd = (int*)arg_v;

    int option;
    char input[MESSAGE_SIZE]; // structure: 0:MESSAGE

    while (1) {
        bzero(input, MESSAGE_SIZE);
        if (read(*newsockfd, input, MESSAGE_SIZE) < 0) {
            error("Error reading from socket");
            continue;
        } else {
            option = input[0];
            printf("Option: %c\n", option);

            const char* message = &input[2];
             // Todo: Since multiple threads will access the following function simultaneously, you may synchronize the access
            operation_args* args = get_op_args(*newsockfd, message);

            switch (option) {
                case '0':
                    get(args);
                    break;
                case '1':
                    put(args);
                    break;
                case '2':
                    delete(args);
                    break;

            }

        }

    }

    return NULL;
}

void* get(void* arg_v) {

    printf("GET\n");

    operation_args* args = (operation_args*)arg_v;


    const char* filename = args->filename;
    int socket = args->newsockfd;

    FILE* file = fopen(filename, "r");

    if (!file) {
        send(socket, &NOT_FOUND, 1, 0);
    }
    else {
        send(socket, &OK, 1, 0);
        // Determine file size
        fseek(file, 0L, SEEK_END);
        size_t file_size = ftell(file);
        fseek(file, 0L, SEEK_SET);

        // Send file size to client
        send(socket, &file_size, sizeof(file_size), 0);

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

        fclose(file);
    }
    return 0;
}

/*
 * If you read this code and say "What a stupidity!" Check out comments in the client code.
 * I know it's inefficient. I know I should only transfer the changes in the file.
 * I don't care.
 * I'm not building an efficient filesystem or change tracker.
 * I'm learning to send stuff.
 *
 * BTW Code logic is same in the GET method of client.
 */
void* put(void* arg_v) {

    printf("PUT\n");

    operation_args* args = (operation_args*)arg_v;

    const char* filename = args->filename;
    int socket = args->newsockfd;

    // Receive confirmation from client. Again, kinda silly but acceptable for simplicity.
    char status[1];
    recv(socket, status, 1, 0);
    if (status[0] == NOT_FOUND) {
        return 0;
    }

    // Receive file size from client.
    size_t file_size;
    if (recv(socket, &file_size, sizeof(file_size), 0) <= 0) {
        printf("Receive error");
        status[0] = INTERNAL_ERROR;
        send(socket, status, 1, 0);
        return 0;
    }

    // Send success
    status[0] = OK;
    send(socket, status, 1, 0);

    // HELL YEA STUPID CODE BEGINS
    // DELETE THE FILE, RECREATE IT AND WRITE NEW STUFF
    // BOOM

    FILE* file = fopen(filename, "w+"); // OFC not that stupid, truncate it instead of deleting and recreating.
    if (!file) {
        printf("Failed to open file");
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
    return 0;
}

void* delete(void* arg_v) {

    printf("DELETE\n");

    operation_args* args = (operation_args*)arg_v;

    const char* filename = args->filename;
    int socket = args->newsockfd;

    if (access(filename, F_OK) != -1) {
        if (remove(filename) == 0) {
            send(socket, &OK, 1, 0);
        } else {
            send(socket, &INTERNAL_ERROR, 1, 0);
        }
        return 0;
    }
    send(socket, &NOT_FOUND, 1, 0);
    return 0;

}

operation_args* get_op_args(int sockfd, const char* filename) {
    if (ht_contains(&table, (void*)filename)) {
        return &HT_LOOKUP_AS(operation_args, &table, (void*)filename);
    }
    operation_args* args = (operation_args*) malloc(sizeof(operation_args));
    if (!args) {
        // Todo: Handle memory allocation failure -> maybe send INTERNAL_ERROR
        return NULL;
    }

    // Allocate memory for semaphores
    args->write_lock = malloc(sizeof(sem_t));
    args->read_lock = malloc(sizeof(sem_t));
    args->no_readers = malloc(sizeof(sem_t));
    args->no_writers = malloc(sizeof(sem_t));

    // Check if memory allocation for semaphores was successful
    if (!args->write_lock || !args->read_lock || !args->no_readers || !args->no_writers) {
        // Handle memory allocation failure for semaphores
        free(args->write_lock);
        free(args->read_lock);
        free(args->no_readers);
        free(args->no_writers);
        free(args);
        return NULL;
    }

    // initialize semaphores
    sem_init(args->no_readers, 0, 1);
    sem_init(args->no_writers, 0, 1);
    sem_init(args->read_lock, 0, 1);
    sem_init(args->write_lock, 0, 1);

    // initialize other fields
    args->newsockfd = sockfd;
    args->filename = filename;

    ht_insert(&table, (void*)filename, args);
    return args;
}

