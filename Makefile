CFLAGS=-O0 -DDEBUG -g -fsanitize=address

all: picohttpparser.o server.o threadpool.o sem.o
	gcc -o dualserver $(CFLAGS) picohttpparser.o server.o threadpool.o sem.o

sem.o: src/sem.c
	gcc -c $(CFLAGS) src/sem.c

picohttpparser.o: picohttpparser-1.2/picohttpparser.c
	gcc -c $(CFLAGS) picohttpparser-1.2/picohttpparser.c

server.o: src/server.c
	gcc -c $(CFLAGS) src/server.c

threadpool.o: src/threadpool.c
	gcc -c $(CFLAGS) src/threadpool.c

clean:
	rm -f *.o a.out

.PHONY: clean
