/* PThread Creation Quiz 1 */ 

#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 4

void *hello (void *arg) { /* thread main */
	printf("Hello Thread\n");
	return 0;
}

int main (void) {
	int i;
	pthread_t tid[NUM_THREADS];
	
	for (i = 0; i < NUM_THREADS; i++) { /* create/fork threads */
		pthread_create(&tid[i], NULL, hello, NULL);
	}
	
	for (i = 0; i < NUM_THREADS; i++) { /* wait/join threads */
		pthread_join(tid[i], NULL);
        // if not join to main thread, main can execute
        // before some threads. Thus, some of the threads may
        //not work
	}
	return 0;
}
