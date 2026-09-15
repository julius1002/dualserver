#include "../picohttpparser-1.2/picohttpparser.h"
#include <unistd.h>
#include <string.h>

struct http_request {
	int conn;
	const char *method, *path;
	int minor_version; // HTTP 1
	size_t method_len, path_len, num_headers, body_len;
	struct phr_header headers[50];
	char *body;
	struct fd_set *master;
	int wakeup_fd;
};

struct http_response {
        int status;
        const char *reason;
        struct phr_header headers[50];
        size_t num_headers;
        char *body;
        size_t body_len;
};

char *serialize(struct http_response *res, size_t *raw_len);
