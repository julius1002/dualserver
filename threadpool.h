#include "http.h"
#include <sys/queue.h>

static struct entry {
	STAILQ_ENTRY(entry) entries;
	void *data;
} *n1, *n2, *n3, *np;

void init_threadpool(int num_threads, int queue_cap, void * (* handler)(void *));

int submit_to_threadpool(struct http_request *req);

void init_queue(int cap);

void threadpool_enqueue(void *data);

struct entry *threadpool_dequeue();
