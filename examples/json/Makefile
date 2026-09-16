main: ../../dualserver.a ./cJSON-1.7.19/libcjson.a main.c
	gcc -g main.c ../../dualserver.a cJSON-1.7.19/libcjson.a

../../dualserver.a: ../../src/server.c ../../src/sem.c ../../src/threadpool.c ../../include/http.h ../../include/server.h ../../include/threadpool.h
	$(MAKE) static -C ../..
