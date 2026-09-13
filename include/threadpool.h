#include "http.h"
#include <sys/queue.h>

struct entry {
	STAILQ_ENTRY(entry) entries;
	void *data;
};

struct threadpool_arg {
	void *(*thread_handle)(void *arg);
	struct {
		char *path;
		size_t path_len;
		char * (*request_handler)(struct http_request *req);
	} *handler_mappings;
	size_t num_mappings;
};

void init_threadpool(int num_threads, int queue_cap, void *arg);

int submit_to_threadpool(struct http_request *req);

void init_queue(int cap);

void threadpool_enqueue(void *data);

struct entry *threadpool_dequeue();
