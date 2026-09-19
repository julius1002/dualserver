#include "http.h"
#include <sys/queue.h>

struct entry {
	STAILQ_ENTRY(entry) entries;
	void *data;
};

struct handler_mapping {
	const char *path;
	size_t path_len;
	RequestHandler request_handler;
};

struct threadpool_arg {
	void *(*thread_handle)(void *arg);
	size_t num_mappings;
	struct handler_mapping *handler_mappings;
        struct dualserver *dserver;
};

void init_threadpool(int num_threads, int queue_cap, void *arg);

int submit_to_threadpool(struct http_request *req);

void init_queue(int cap);

void threadpool_enqueue(void *data);

struct entry *threadpool_dequeue();
