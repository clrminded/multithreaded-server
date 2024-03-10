//
// client.c: A very, very primitive HTTP client.
// 
// To run, try: 
//      client hostname portnumber filename
//
// Sends one HTTP request to the specified HTTP server.
// Prints out the HTTP response.
//
// For testing your server, you will want to modify this client.  
// For example:
// You may want to make this multi-threaded so that you can 
// send many requests simultaneously to the server.
//
// You may also want to be able to request different URIs; 
// you may want to get more URIs from the command line 
// or read the list from a file. 
//
// When we test your server, we will be using modifications to this client.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "io_helper.h"

#define MAXBUF (8192)

// mutex variable declaration
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// thread function for sending requests and printing responses
void *client_thread(void *arg) 
{
    char **args = (char **) arg;
    char *host = args[0];
    int port = atoi(args[1]);
    char *filename = args[2];

    int clientfd;

    // open a single connection to the specified host and port
    if((clientfd = open_client_fd_or_die(host, port)) < 0)
    {
        perror("Failed to open client socket");
        pthread_exit(NULL);
    }

    // send HTTP request
    client_send(clientfd, filename);

    // print HTTP response
    client_print(clientfd);

    // close connection
    close_or_die(clientfd);

    pthread_exit(NULL);
}



//
// Send an HTTP request for the specified file 
//
void client_send(int fd, char *filename) 
{
    char buf[MAXBUF];
    char hostname[MAXBUF];

    gethostname_or_die(hostname, MAXBUF);
    
    /* Form and send the HTTP request */
    sprintf(buf, "GET %s HTTP/1.1\n", filename);
    sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
    write_or_die(fd, buf, strlen(buf));
}

//
// Read the HTTP response and print it out
//
void client_print(int fd) {
    char buf[MAXBUF];  
    int n;
    
    // Read and display the HTTP Header 
    n = readline_or_die(fd, buf, MAXBUF);
    while (strcmp(buf, "\r\n") && (n > 0)) 
    {
        printf("Header: %s", buf);
        n = readline_or_die(fd, buf, MAXBUF);
        
        // If you want to look for certain HTTP tags... 
        // int length = 0;
        //if (sscanf(buf, "Content-Length: %d ", &length) == 1) {
        //    printf("Length = %d\n", length);
        //}
    }
    
    // Read and display the HTTP Body 
    n = readline_or_die(fd, buf, MAXBUF);
    while (n > 0) 
    {
        printf("%s", buf);
        n = readline_or_die(fd, buf, MAXBUF);
    }
}



int main(int argc, char *argv[]) 
{
    char *host, *filename;
    int port, num_threads;
    
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <host> <port> <filename> <num_threads>\n", argv[0]);
        exit(1);
    }
    
    host = argv[1];
    port = atoi(argv[2]);
    filename = argv[3];
    num_threads = atoi(argv[4]);

    // initialize mutex
    if(pthread_mutex_init(&mutex, NULL) != 0) 
    {
        perror("Mutex initilization failed");
        exit(EXIT_FAILURE);
    }

    // create array of thread IDs
    pthread_t threads[num_threads];

    for(int i = 0; i < num_threads; i++) 
    {
        char *args[] = {host, argv[2], filename};
        if(pthread_create(&threads[i], NULL, client_thread, (void *)args) != 0)
        {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // join the threads
    for(int i = 0; i < num_threads; i++) 
    {
        if(pthread_join(threads[i], NULL) != 0) 
        {
            perror("Thread join failed");
            exit(EXIT_FAILURE);
        }
    }
    
    
    // Destroy mutex
    pthread_mutex_destroy(&mutex);


    return 0;
}
