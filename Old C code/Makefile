CC=g++
CFLAGS= -g -Wall

all: proxy_with_cache proxy_without_cache

proxy_with_cache: proxy_server_with_cache.o proxy_parse.o
	$(CC) $(CFLAGS) -o proxy_with_cache proxy_server_with_cache.o proxy_parse.o -lpthread

proxy_without_cache: proxy_server_without_cache.o proxy_parse.o
	$(CC) $(CFLAGS) -o proxy_without_cache proxy_server_without_cache.o proxy_parse.o -lpthread

proxy_server_with_cache.o: proxy_server_with_cache.c
	$(CC) $(CFLAGS) -c proxy_server_with_cache.c -lpthread

proxy_server_without_cache.o: proxy_server_without_cache.c
	$(CC) $(CFLAGS) -c proxy_server_without_cache.c -lpthread

proxy_parse.o: proxy_parse.c
	$(CC) $(CFLAGS) -c proxy_parse.c -lpthread

clean:
	rm -f proxy_with_cache proxy_without_cache *.o

tar:
	tar -cvzf ass1.tgz proxy_server_without_cache.c README Makefile proxy_parse.c proxy_parse.h