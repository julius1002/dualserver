main: ../../dualserver.a
	gcc -g main.c ../../dualserver.a

../../dualserver.a: ../../src/server.c ../../src/sem.c ../../src/threadpool.c ../../include/http.h ../../include/server.h ../../include/threadpool.h
	$(MAKE) static -C ../..
