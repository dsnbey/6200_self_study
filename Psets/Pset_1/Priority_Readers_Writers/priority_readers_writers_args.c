# include <stdio.h>
# include <pthread.h>
# include <stdlib.h>
# include <unistd.h>

int global_var = 0;
int curr_reader = 0;
int curr_writer = 0;

int operation_count;
int reader_count, writer_count;


pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_reader = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_writer = PTHREAD_COND_INITIALIZER;

void* read_var(void* param);
void* write_var(void*param);

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Enter arg1=reader count, arg2=writer count, arg3=total operation to make");
        return 1;
    }

    // initialize arguments
    reader_count = atoi(argv[1]);
    writer_count = atoi(argv[2]);
    operation_count = atoi(argv[3]);

    // create threads
    pthread_t readers[reader_count];
    pthread_t writers[writer_count];

    for (int i = 0; i < writer_count; i++) {
        pthread_create(&writers[i], NULL, write_var, &operation_count);
    }

    for (int i = 0; i < reader_count; i++) {
        pthread_create(&readers[i], NULL, read_var, &operation_count);
    }

    // join threads

    for (int i = 0; i < reader_count; i++) {
        pthread_join(readers[i], NULL);
    }

    for (int i = 0; i < writer_count; i++) {
        pthread_join(writers[i], NULL);
    }

    printf("Work done!\n");
    return 0;

}

void *read_var(void *param) {
    int ops = *((int*)param);

    for (int i = 0; i < ops; i++) {
        pthread_mutex_lock(&m);
        printf("Reader try: \n");
        while (curr_writer != 0) {
            pthread_cond_wait(&cond_reader, &m);
        }
        curr_reader++;
        pthread_mutex_unlock(&m);

        unsigned int microseconds = 300000; // 0.3 seconds
        usleep(microseconds);
        printf("READ: Number of current readers = %d, Value read = %d\n", curr_reader, global_var);

        pthread_mutex_lock(&m);
        curr_reader--;
        if (curr_reader == 0 && curr_writer > 0) {
            pthread_cond_signal(&cond_writer);
        }
        pthread_mutex_unlock(&m);
    }

    printf("Reader done!\n");
    return 0;
}

void *write_var(void *param) {
    int ops = *((int*)param);

    for (int i = 0; i < ops; i++) {
        pthread_mutex_lock(&m);
        printf("Writer try: \n");
        while (curr_reader != 0 || curr_writer != 0) {
            pthread_cond_wait(&cond_writer, &m);
        }
        curr_writer++;

        unsigned int microseconds = 300000; // 0.3 seconds
        usleep(microseconds);
        global_var++;
        printf("WRITE: Number of current writers = %d, New value = %d\n", curr_writer, global_var);
        curr_writer--;

        if (curr_writer == 0) {
            pthread_cond_broadcast(&cond_reader);
        }
        pthread_mutex_unlock(&m);
    }

    printf("Writer done!\n");
    return 0;
}

