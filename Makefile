CFLAGS=-O0 -DDEBUG -g
#-fsanitize=address

static: picohttpparser.o server.o threadpool.o sem.o http.o
	ar rcs dualserver.a picohttpparser.o server.o threadpool.o sem.o http.o

all: picohttpparser.o server.o threadpool.o sem.o http.o
	gcc -c $(CFLAGS) picohttpparser.o server.o threadpool.o sem.o http.o

http.o: src/http.c
	gcc -c $(CFLAGS) src/http.c

sem.o: src/sem.c
	gcc -c $(CFLAGS) src/sem.c

picohttpparser.o: picohttpparser-1.2/picohttpparser.c
	gcc -c $(CFLAGS) picohttpparser-1.2/picohttpparser.c

server.o: src/server.c
	gcc -c $(CFLAGS) src/server.c

threadpool.o: src/threadpool.c
	gcc -c $(CFLAGS) src/threadpool.c

clean:
	rm -f *.o a.out *.a

.PHONY: clean
