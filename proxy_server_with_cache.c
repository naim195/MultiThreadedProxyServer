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
#define MAX_BYTES 4096

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
int proxy_socketId;
pthread_t tid[MAX_CLIENTS]; // thread ids array to store ids of clients
sem_t semaphore;
pthread_mutex_t lock; // to lock cache

cache_element *head;
int cache_size;

void *thread_fn(void* socketNew){
    sem_wait(&semaphore);
    int p;
    sem_getvalue(&semaphore, p);
    printf("Semaphore value is:%d\n", p);
	int *t = (int *)socketNew;
	int socket = *t;
	int bytes_send_client, len;
	char *buffer = (char *)calloc(MAX_BYTES, sizeof(char));
	bzero(buffer, MAX_BYTES);
}

int main(int argc, char *argv[])
{
    int client_socketId, client_len;
    struct sockaddr_in server_addr, client_addr;
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

     //creating the proxy socket
	proxy_socketId = socket(AF_INET, SOCK_STREAM, 0);

	if( proxy_socketId < 0)
	{
		perror("Failed to create socket.\n");
		exit(1);
	}

	int reuse =1;
	if (setsockopt(proxy_socketId, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) 
        perror("setsockopt(SO_REUSEADDR) failed\n");

	bzero((char*)&server_addr, sizeof(server_addr));  
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_number); // assigning port to the Proxy
	server_addr.sin_addr.s_addr = INADDR_ANY; // any available address assigned

    // bind the socket
	if( bind(proxy_socketId, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 )
	{
		perror("Port is not available\n");
		exit(1);
	}
	printf("Binding on port: %d\n",port_number);

    // Proxy socket listening to the requests
	int listen_status = listen(proxy_socketId, MAX_CLIENTS);

	if(listen_status < 0 )
	{
		perror("Error while Listening !\n");
		exit(1);
	}

	int i = 0; // Iterator for thread_id (tid) and Accepted Client_Socket for each thread
	int Connected_socketId[MAX_CLIENTS];   // This array stores socket descriptors of connected clients

    // Infinite Loop for accepting connections
	while(1)
	{
		
		bzero((char*)&client_addr, sizeof(client_addr));			// Clears struct client_addr
		client_len = sizeof(client_addr); 

        // accepting the connections
		client_socketId = accept(proxy_socketId, (struct sockaddr*)&client_addr,(socklen_t*)&client_len);	// Accepts connection
		if(client_socketId < 0)
		{
			fprintf(stderr, "Unable to connect !\n");
			exit(1);
		}
		else{
			Connected_socketId[i] = client_socketId; // Storing accepted client into array
		}

		// Getting IP address and port number of client
		struct sockaddr_in* client_pt = (struct sockaddr_in*)&client_addr;
		struct in_addr ip_addr = client_pt->sin_addr;
		
		char str[INET_ADDRSTRLEN];  // INET_ADDRSTRLEN: Default ip address size
		inet_ntop( AF_INET, &ip_addr, str, INET_ADDRSTRLEN );
		printf("Client is connected with port number: %d and ip address: %s \n",ntohs(client_addr.sin_port), str);
		//printf("Socket values of index %d in main function is %d\n",i, client_socketId);
		pthread_create(&tid[i],NULL,thread_fn, (void*)&Connected_socketId[i]); // creating a thread for each client accepted
		i++; 
	}
	close(proxy_socketId);									// close socket
 	return 0;
}