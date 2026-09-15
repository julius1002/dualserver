#include "../../include/server.h"

#define GREETINGS_RESPONSE "HTTP/1.0 200 OK\r\nContent-Length: 10\r\n\r\nGreetings!"

char *greetings_handler(struct http_request *req, struct http_response *res) 
{
	printf("body is: %s\n", req->body);
	return GREETINGS_RESPONSE;
}

int main() 
{
	struct dualserver dserver;
	init_dualserver(&dserver);
	add_handler_mapping("/greetings", greetings_handler, &dserver);
	launch_dualserver(&dserver);
	return 0;
}
