main: ../../libdualserver.a ./cJSON-1.7.19/libcjson.a main.c
	gcc -g main.c -L../.. -I../../include cJSON-1.7.19/libcjson.a -ldualserver

../../dualserver.a: ../../src/server.c ../../src/sem.c ../../src/threadpool.c ../../include/http.h ../../include/server.h ../../include/threadpool.h
	$(MAKE) static -C ../..
