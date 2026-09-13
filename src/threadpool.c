#include "../include/threadpool.h"
#include <pthread.h>
#include "../include/sem.h"
#include <stdlib.h>

STAILQ_HEAD(stailhead, entry) head;

struct threadpool {
	int num_threads;
	struct queue {
		my_sem_t taken_slots, free_slots;
		struct stailhead *headp;
		pthread_mutex_t mutex;
		struct stailhead head;
	} queue;
	pthread_t threads[];
} tp;

void init_threadpool(int num_threads, int queue_cap, void *arg) {
	tp.num_threads = num_threads;
	init_queue(queue_cap);
        struct threadpool_arg *tp_arg = (struct threadpool_arg *) arg;
	for(int i = 0; i < num_threads; i++) {
		pthread_create(&tp.threads[i], NULL, tp_arg->thread_handle, tp_arg);
	}
}

void init_queue(int cap) {
	pthread_mutex_init(&tp.queue.mutex, NULL);
        STAILQ_INIT(&tp.queue.head);
	my_sem_init(&tp.queue.free_slots, cap);
	my_sem_init(&tp.queue.taken_slots, 0);
	tp.queue.head = (struct stailhead) STAILQ_HEAD_INITIALIZER(head);
}

struct entry *threadpool_dequeue() {
	my_sem_wait(&tp.queue.taken_slots);

	if(0 > pthread_mutex_lock(&tp.queue.mutex)) {
		perror("pthread_mutex_lock");
		return NULL;
	}

	struct entry *fst = STAILQ_FIRST(&tp.queue.head);
	STAILQ_REMOVE_HEAD(&tp.queue.head, entries);
	if(0 > pthread_mutex_unlock(&tp.queue.mutex)) {
		perror("pthread_mutex_unlock");
		return NULL;
	}
	my_sem_post(&tp.queue.free_slots);
	return fst;
}

// check if this pointer is the real request pointer
void threadpool_enqueue(void *data) {
	struct http_request *req = (struct http_request *) data;
	my_sem_wait(&tp.queue.free_slots);
	struct entry *item = malloc(sizeof(struct entry));      /* Insert at the head. */
	item->data = (void *) data;
	pthread_mutex_lock(&tp.queue.mutex);
	STAILQ_INSERT_HEAD(&tp.queue.head, item, entries);
	pthread_mutex_unlock(&tp.queue.mutex);
	my_sem_post(&tp.queue.taken_slots);
}

