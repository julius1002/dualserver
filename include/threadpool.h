#include "http.h"
#include <sys/queue.h>

struct entry {
	STAILQ_ENTRY(entry) entries;
	void *data;
};

struct threadpool_arg {
	char *(*request_handler)(struct http_request *req);
	void *(*thread_handle)(void *arg);
};

void init_threadpool(int num_threads, int queue_cap, void *arg);

int submit_to_threadpool(struct http_request *req);

void init_queue(int cap);

void threadpool_enqueue(void *data);

struct entry *threadpool_dequeue();
