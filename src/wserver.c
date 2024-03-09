#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "request.h"
#include "io_helper.h"

char default_root[] = ".";

//
// ./wserver [-d <basedir>] [-p <portnum>] 
// 

// Thread function for handling client requests
void *handle_request(void *arg) 
{
	int conn_fd = *((int *)arg);
	request_handle(conn_fd);
	close_or_die(conn_fd);
	free(arg); // free memory allowed for the connection file descriptor
	return NULL;
}

int main(int argc, char *argv[]) {
    int c;
    char *root_dir = default_root;
    int port = 10000;
    
    while ((c = getopt(argc, argv, "d:p:")) != -1)
	switch (c) {
	case 'd':
	    root_dir = optarg;
	    break;
	case 'p':
	    port = atoi(optarg);
	    break;
	default:
	    fprintf(stderr, "usage: wserver [-d basedir] [-p port]\n");
	    exit(1);
	}

    // run out of this directory
    chdir_or_die(root_dir);

    // now, get to work
    int listen_fd = open_listen_fd_or_die(port);

	// critical section where threads will have to be implemented
    while (1) 
	{
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		// Allocate memory to pass the file descriptor to the thread
		int *conn_fd_ptr = malloc(sizeof(int));
		*conn_fd_ptr = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);

		pthread_t tid;
		
		if(pthread_create(&tid, NULL, handle_request, conn_fd_ptr) != 0)
		{
			perror("pthread_create");
			// close connection in case thread creation failure
			close_or_die(*conn_fd_ptr);
			// free memory allocated for the connection file descriptor
			free(conn_fd_ptr);
		}

		// detach the thread to avoid memory leaks
		pthread_detach(tid);
    }

    return 0;
}


    


 
