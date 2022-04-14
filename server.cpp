/*
** server.c -- a stream socket server demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <pthread.h> // threads
#include <tbb/mutex.h>

#include "stack.h"
#include <vector>
#include <iostream>


#define PORT "3491"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold
#include <tbb/mutex.h>


bool online = true;
//Stack stack;
//struct stack *stack = (struct stack*)malloc(sizeof(struct stack));
struct stack stack;
//pthread_t *thread_id = (pthread_t*)malloc(sizeof(pthread_t)*BACKLOG); // up to 10 clients
pthread_t thread_id[BACKLOG];   // UP TO 10 CONNECTIONS
int new_fd[BACKLOG];            // UP TO 10 CONNECTIONS
tbb::mutex mutex;
int sockfd;  // listen on sock_fd, new connection on new_fd


/*
HERE We need to add the stack's implementation and function
*/

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *threadfunc(void *newfd) {
    /*
    This is the function that manages every client. 
    These are its functions::
        - A while true loop that receives from the client data (with recv() function). 
        - Once we receive data from the client we need to break it into a string and manage it:
            - TOP will run the stack's top function and send data to the client (with send() function)
            - PUSH will run the stack's push function.
            - POP will run the stack's pop function. 
    */
    
    int new_fd = *(int*)newfd; 
    pthread_detach(pthread_self());
    int numbytes;  
    char buf[1024];
    bool connected = true;
    while (connected&&online) {
      
        numbytes = recv(new_fd, buf, sizeof(buf), 0);
        if (numbytes <=0) {
            perror("recv");
            connected = false;pthread_exit(NULL);	
            break;
        }
        *(buf+numbytes) = '\0';
         
        if (!strcmp(buf, "PUSH")) {
            tbb::mutex::scoped_lock lock(mutex);
            //pthread_mutex_lock(&mutex);
            numbytes = recv(new_fd, buf, sizeof(buf), 0);
            if (numbytes <=0) {
                perror("recv");
                connected = false;
                break;
            }
           
            printf("DEBUG: Pushing %s\n", buf);
            //stack.push(buf);
            push(&stack, buf);
            //printf("PUSHED %s sussfully\n", stack.ptr->text);
            //pthread_mutex_unlock(&mutex);
        } else if (!strcmp(buf, "TOP")) {
            printf("the top is new %s\n", stack.ptr->text);
            if (send(new_fd, top(stack), 1024, 0) == -1)  {
                perror("send");
            }
           
        } else if (!strcmp(buf, "POP")) {
            tbb::mutex::scoped_lock lock(mutex);
            //pthread_mutex_lock(&mutex);
            if (pop(&stack)) {
                if (send(new_fd, "DEBUG: popped succeeded", 1024, 0) == -1)  {
                    perror("send");
                }
            } else {
                if (send(new_fd, "DEBUG: popped failed", 1024, 0) == -1)  {
                    perror("send");
                    
                }
            }
            //pthread_mutex_unlock(&mutex);
        }
        
   }
    //  pthread_mutex_unlock(&mutex);
    printf("closed a client socket\n");
    close(new_fd);
    //pthread_exit(NULL);	
    //return newfd;
}

void sig_handler(int signum)
{

    if (signum == SIGINT) {
        printf("program terminating\n");
        online = false;
        for (size_t i = 0; i < BACKLOG; i++)
        {
            if (send(new_fd[i], "CLOSEING", 1024, 0) == -1)  {
                perror("send");
            }
            close(new_fd[i]);
        }
        
        while (!stack.isEmpty) {
            pop(&stack);
        }
        close(sockfd);
        printf("program terminated gracefully");
        exit(1);
    }

}
int main(void)
{
   
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    int j = 0;
    signal (SIGINT,sig_handler);
    
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd[j] = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd[j] == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);
        
        pthread_create(&thread_id[j%BACKLOG], NULL, threadfunc, &new_fd[j]);
        j++;
    }

    return 0;
}
