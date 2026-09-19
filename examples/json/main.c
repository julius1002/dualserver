#include <server.h>
#include "./cJSON-1.7.19/cJSON.h"

#define GREETINGS_RESPONSE "HTTP/1.0 200 OK\r\nContent-Length: 10\r\n\r\nGreetings!"

void greetings_handler(struct http_request *req, struct http_response *res)
{
	printf("body: %s\n", req->body);

	cJSON *json = cJSON_Parse(req->body);
	cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "hello");

	if (cJSON_IsString(name) && (name->valuestring != NULL))
	{
		printf("value for key: hello = \"%s\"\n", name->valuestring);
	}

	res->status = 200;
	res->reason = "OK";
	res->num_headers = 0;
	res->body = "OK";
	res->body_len = 2;
}

int main() 
{
	struct dualserver dserver;
	init_dualserver(&dserver, 3000);
	add_handler_mapping("/greetings", greetings_handler, &dserver);
	launch_dualserver(&dserver);
	return 0;
}
