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

// define the thread arguments
typedef struct {
    char *host;
    int port;
    char *filename;
} ThreadArgs;

//
// Send an HTTP request for the specified file 
//
void client_send(void *arg) 
{
    ThreadArgs *args = (ThreadArgs *)arg;
    int clientfd;
    char buf[MAXBUF];
    
    clientfd = open_client_fd_or_die(args->host, args->port);
    
    /* Form and send the HTTP request */
    sprintf(buf, "GET %s HTTP/1.1\n", args->filename);
    sprintf(buf, "%shost: %s\n\r\n", buf, args->host);
    write_or_die(clientfd, buf, strlen(buf));

    // free memory allocated for the arguments
    free(args); 
    client_print(clientfd);
    close_or_die(clientfd);
    return NULL;
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
    char *filename;
    int port;
    int num_threads;
    
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <host> <port> <filename> <num_threads>\n", argv[0]);
        exit(1);
    }
    
    char *host = argv[1];
    port = atoi(argv[2]);
    filename = argv[3];
    num_threads = argv[4];

    pthread_t threads[num_threads];

    for(int i = 0; i < num_threads; i++) 
    {
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        args->host = host;
        args->port = port;
        args->filename = filename;

        if(pthread_create(&threads[i], NULL, client_send, args) != 0)
        {
            perror("pthread_create");
            exit(1);
        }
    }

    // join the threads
    for(int i = 0; i < num_threads; i++) 
    {
        pthread_join(threads[i], NULL);
    }
    
    
    exit(0);
}
