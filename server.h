#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "threadpool.h"

#define PORT "3000"

#define RESPONSE "HTTP/1.0 200 OK\r\nContent-Length: 12\r\n\r\nHello world!"
