// 
// wserver.c: A Multithreaded HTTP server
//
// To Run:
// 		../p4a/runtests -c
//
// Passed 21 of 21 tests.
// Overall 21 of 21
// Points 100 of 100

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "request.h"
#include "io_helper.h"

char default_root[] = "."; // Default root directory
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex variable declaration used for locks

int buffer_size = 8192; // global variable for the buffer size

/* Thread function for handling connections
 * @params:
 * - arg: pointer to an integer representing the listening socket file descriptor
 * @return: void pointer 
 */
void *connection_handler(void *arg) 
{
	int listen_fd = *((int *) arg); // Listening socket file descriptor
	while(1)
	{
		struct sockaddr_in client_addr; // Client address structure of type sockaddr_in
		int client_len = sizeof(client_addr); // Length of the client address structure
		int conn_fd; // Connection file descriptor

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

int main(int argc, char *argv[]) 
{
    int c; // Optional character used for parsing
    char *root_dir = default_root; // Root directory for serving files
    int port = 10000; // Default port number
	int num_threads = 4; // Default number of threads
	int buffer_size = 0; // Default buffer size
    
	// Initialize the mutex, otherwise exit from server
	if(pthread_mutex_init(&mutex, NULL) != 0)
	{
		perror("Mutex initialization failed");
		exit(EXIT_FAILURE);
	}

	// Parse command-line arguments
    while ((c = getopt(argc, argv, "d:p:t:b:")) != -1)
	{
		switch (c) 
		{
			case 'd':
				root_dir = optarg; // Set root directory from command-line, otherwise use default value
				break;
			case 'p':
				port = atoi(optarg); // Set port number from command-line, otherwise use default value
				break;
			case 't':
				num_threads = atoi(optarg); // Set number of threads from command-line, otherwise use default value
				if(num_threads <= 0) // Check if number of threads is valid
				{
					fprintf(stderr, "Invalid number of threads\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'b':
				buffer_size = atoi(optarg); // Set buffer_size from command-line, otherwise use default value
				if(buffer_size <= 0) // Check is buffer_size is valid
				{
					fprintf(stderr, "Invalid buffer_size\n");
					exit(EXIT_FAILURE);
				}
				break;
			default:
				fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t num_threads] [-b buffer-size]\n");
				exit(EXIT_FAILURE);
		}
	}

    // Run out of this directory
    chdir_or_die(root_dir); // Change the current working directory

    // Now, get to work
    int listen_fd = open_listen_fd_or_die(port); // Open listen socket

	// Create an array of pthread_t to store thread IDs
	pthread_t threads[num_threads];

	// Create mutiple threads to handle connections
	for(int i = 0; i < num_threads; i++)
	{
		if(pthread_create(&threads[i], NULL, connection_handler, (void *) &listen_fd) != 0)
		{
			perror("Thread creation failed"); // Print error message if thread create fails
			exit(EXIT_FAILURE); // Exit with failure status
		}
	}

	// Join threads (wait for them to finish)
	for(int i = 0; i < num_threads; i++) 
	{
		if(pthread_join(threads[i], NULL) != 0) // Join thread
		{
			perror("Thread join failed"); // Print error message if thread fails
			exit(EXIT_FAILURE); // Exit with failure status
		}
	}

	// Destroy the mutex
	pthread_mutex_destroy(&mutex);

    return 0; // Exit with success status
}


    


 
