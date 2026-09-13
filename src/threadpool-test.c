#include "../include/threadpool.h"
#include <sys/queue.h>
#include <pthread.h>
#include <stdio.h>

void *producer(void *arg) {
	for(int i = 0; i < 100000; i++) {
		threadpool->enqueue((void *)i);
	}
	return NULL;
}

void *consumer(void *arg) {
	for(int i = 0; i < 25000; i++) {
		printf("value: %d thread: %p\n", *((int *) &threadpool->dequeue()->data), pthread_self());
	}
	return NULL;
}

/* Demonstration of the queue being accessed by multiple threads 
 *
 * If the program does not stall, everything is fine.
 *
 **/
int test() {
	init_queue(100);
       
	pthread_t th[5];
        pthread_create(&th[0], NULL, producer, NULL);
        pthread_create(&th[1], NULL, consumer, NULL);
        pthread_create(&th[2], NULL, consumer, NULL);
        pthread_create(&th[3], NULL, consumer, NULL);
        pthread_create(&th[4], NULL, consumer, NULL);

	pthread_join(th[0], NULL);
	pthread_join(th[1], NULL);
	pthread_join(th[2], NULL);
	pthread_join(th[3], NULL);
	pthread_join(th[4], NULL);

	return 0;
}

int main() {
	test();
	return 0;
}
