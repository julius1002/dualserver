#include "../../include/server.h"
#include <string.h>

#define GREETINGS_RESPONSE "HTTP/1.0 200 OK\r\nContent-Length: 10\r\n\r\nGreetings!"

void greetings_handler(struct http_request *req, struct http_response *res) 
{
	printf("body is: %s\n", req->body);
	res->body = "Greetingssss!";
}

int main() 
{
	struct dualserver dserver;
	init_dualserver(&dserver, 3000);
        init_static_files(&dserver, "./public");
	add_handler_mapping("/greetings", greetings_handler, &dserver);
	launch_dualserver(&dserver);
	return 0;
}
