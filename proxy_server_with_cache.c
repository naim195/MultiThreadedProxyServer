#include "proxy_parse.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_CLIENTS 10

typedef struct cache_element cache_element;

struct cache_element
{
    char *data;            // for response
    int len;               // length of data
    char *url;             // url of request
    time_t lru_time_track; // time at which element was last accessed
    cache_element *next;   // linked list implementation so points to next element
};

// define functions
cache_element *find(char *url);
int add_cache_element(char *data, int size, char *url);
void remove_cache_element();

int port_number = 8080;
int proxySocketId;
pthread_t tid[MAX_CLIENTS]; // thread ids array to store ids of clients
sem_t semaphore;
pthread_mutex_t lock; // to lock cache

cache_element *head;
int cache_size;

int main(int argc, char *argv[])
{
    int client_socketId, client_len;
    struct sockaddr server_addr, client_addr;
    sem_init(&semaphore, 0, MAX_CLIENTS);
    pthread_mutex_init(&lock, NULL);
    if(argv==2){
        port_number = atoi(argv[1]);
    }
    else{
        printf("Too few arguments\n");
        exit(1);
    }

    printf("Starting proxy server at port, %d\n", port_number);
    
}