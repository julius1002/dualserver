CFLAGS=-O0 -DDEBUG -g -fsanitize=address

all: picohttpparser.o server.o threadpool.o sem.o
	gcc -o dualserver $(CFLAGS) picohttpparser.o server.o threadpool.o sem.o

sem.o: sem.c
	gcc -c $(CFLAGS) sem.c

picohttpparser.o: picohttpparser-master/picohttpparser.c
	gcc -c $(CFLAGS) picohttpparser-master/picohttpparser.c

selectserver.o: server.c
	gcc -c $(CFLAGS) server.c

threadpool.o: threadpool.c
	gcc -c $(CFLAGS) threadpool.c

clean:
	rm -f *.o a.out

.PHONY: clean
