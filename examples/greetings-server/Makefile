main: ../../dualserver.a main.c
	gcc -g $(CFLAGS) main.c -L../.. -I../../include -ldualserver
 
../../dualserver.a: ../../src/server.c ../../src/http.c ../../src/sem.c ../../src/threadpool.c ../../include/http.h ../../include/server.h ../../include/threadpool.h
	$(MAKE) static -C ../..
