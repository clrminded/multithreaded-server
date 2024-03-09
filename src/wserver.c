#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "request.h"
#include "io_helper.h"

char default_root[] = ".";
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// global variable for the buffer size
int buffer_size = 8192;

// Thread function for handling connections (current)
void *connection_handler(void *arg) 
{
	int listen_fd = *((int *) arg);
	while(1)
	{
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd;

		// lock the critical section before accepting connection
		pthread_mutex_lock(&mutex);
		conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
		pthread_mutex_unlock(&mutex);

		// handle request
		request_handle(conn_fd);

		// close connection
		close_or_die(conn_fd);
	}

	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
	int num_threads = 4; // default number of threads
    
	// initialize the mutex
	if(pthread_mutex_init(&mutex, NULL) != 0)
	{
		perror("Mutex initialization failed");
		exit(EXIT_FAILURE);
	}

    while ((c = getopt(argc, argv, "d:p:t:b:")) != -1)
	{
		switch (c) 
		{
			case 'd':
				root_dir = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'b':
				buffer_size = atoi(optarg);
				break;
			default:
				fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t num_threads] [-b buffer-size]\n");
				exit(EXIT_FAILURE);
		}
	}

    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);

	// create an array of pthread_t to store thread IDs
	pthread_t threads[num_threads];

	// create mutiple threads to handle connections
	for(int i = 0; i < num_threads; i++)
	{
		if(pthread_create(&threads[i], NULL, connection_handler, (void *) &listen_fd) != 0)
		{
			perror("Thread creation failed");
			exit(EXIT_FAILURE);
		}
	}

	// join threads (wait for them to finish)
	for(int i = 0; i < num_threads; i++) 
	{
		if(pthread_join(threads[i], NULL) != 0) 
		{
			perror("Thread join failed");
			exit(EXIT_FAILURE);
		}
	}

	// destroy the mutex
	pthread_mutex_destroy(&mutex);


    return 0;
}


    


 
