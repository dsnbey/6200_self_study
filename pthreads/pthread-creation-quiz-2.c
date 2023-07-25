/* A Slightly Less Simple PThreads Example */

#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 4

void *threadFunc(void *pArg) { /* thread main */
	int *p = (int*)pArg;
	int myNum = *p;
	printf("Thread number %d\n", myNum);
	return 0;
}

int main(void) {
	int i;
	pthread_t tid[NUM_THREADS];
	
	for(i = 0; i < NUM_THREADS; i++) { /* create/fork threads */
		pthread_create(&tid[i], NULL, threadFunc, &i);
        // i is globally visible! WHen i changes, all other
        // threads see the new i

        // RACE CONDITION -> A THREAD TRIES TO READ A VALUE
        // WHILE ANOTHER THREAD MODIFIES IT
	}
	
	for(i = 0; i < NUM_THREADS; i++) { /* wait/join threads */
		pthread_join(tid[i], NULL);
	}
	return 0;
}
