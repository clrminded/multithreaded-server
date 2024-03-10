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

// Mutex variable declaration
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread function for sending requests and printing responses
// @param:
// - arg: void pointer to an array of strings containing host, port, and filename
// @return: void pointer
void *client_thread(void *arg) 
{
    char **args = (char **) arg; // Array of strings containing host, port, and filename
    char *host = args[0];     // Hostname
    int port = atoi(args[1]); // Port number
    char *filename = args[2]; // Filename

    int clientfd;

    // Open a single connection to the specified host and port
    if((clientfd = open_client_fd_or_die(host, port)) < 0)
    {
        perror("Failed to open client socket"); // Print error message if failed to open socket
        pthread_exit(NULL); // Exit thread
    }

    // send HTTP request
    client_send(clientfd, filename);

    // print HTTP response
    client_print(clientfd);

    // close connection
    close_or_die(clientfd);

    pthread_exit(NULL); // Exit thread
}

//
// Send an HTTP request for the specified file 
//
// @params:
// - fd: File descriptor of the socket
// - filename: Name of the file to request
// @return: void
void client_send(int fd, char *filename) 
{
    char buf[MAXBUF]; // Buffer for storing HTTP request
    char hostname[MAXBUF]; // Buffer for storing hostname

    gethostname_or_die(hostname, MAXBUF);
    
    /* Form and send the HTTP request */
    sprintf(buf, "GET %s HTTP/1.1\n", filename);
    sprintf(buf, "%shost: %s\n\r\n", buf, hostname);
    write_or_die(fd, buf, strlen(buf)); // Write HTTP request to socket
}

//
// Read the HTTP response and print it out
//
// @param: 
// - fd: File descriptor of the socket
// @return: void
void client_print(int fd) 
{
    char buf[MAXBUF]; // Buffer for storuing HTTP response
    int n; // Number of bytes read
    
    // Read and display the HTTP Header 
    n = readline_or_die(fd, buf, MAXBUF);
    while (strcmp(buf, "\r\n") && (n > 0)) 
    {
        printf("Header: %s", buf); // Print HTTP header
        n = readline_or_die(fd, buf, MAXBUF);
    }
    
    // Read and display the HTTP Body 
    n = readline_or_die(fd, buf, MAXBUF);
    while (n > 0) 
    {
        printf("%s", buf); // Print HTTP body
        n = readline_or_die(fd, buf, MAXBUF);
    }
}

int main(int argc, char *argv[]) 
{
    char *host, *filename; // Hostname and filename
    int port, num_threads; // Port number and number of threads
    
    if (argc != 6) 
    {
        fprintf(stderr, "Usage: %s <host> <port> <filename> <num_threads>\n", argv[0]);
        exit(1);
    }
    
    host = argv[1];             // Set hostname from command-line argument
    port = atoi(argv[2]);       // Set port number from command-line argument
    filename = argv[3];         // Set filename from command-line argument
    num_threads = atoi(argv[4]);// Set number of threads from command-line argument

    // Initialize mutex
    if(pthread_mutex_init(&mutex, NULL) != 0) 
    {
        perror("Mutex initilization failed"); // Print error message if mutex initialization fails
        exit(EXIT_FAILURE);
    }

    // Create array of thread IDs
    pthread_t threads[num_threads];

    // Create multiple threads to send requests
    for(int i = 0; i < num_threads; i++) 
    {
        char *args[] = {host, argv[2], filename};
        if(pthread_create(&threads[i], NULL, client_thread, (void *)args) != 0) // Arguments to be passed to thread function
        {
            perror("Thread creation failed"); // Print error message if thread creation fails
            exit(EXIT_FAILURE);
        }
    }

    // Join the threads
    for(int i = 0; i < num_threads; i++) 
    {
        if(pthread_join(threads[i], NULL) != 0) 
        {
            perror("Thread join failed"); // Print error message if thread join fails
            exit(EXIT_FAILURE);
        }
    }
    
    // Destroy mutex
    pthread_mutex_destroy(&mutex);

    return 0; // Exit with success status
}