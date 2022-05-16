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

#include <sys/mman.h>

//#include <tbb/mutex.h>

#include "stack.h"
#include <vector>
#include <iostream>


#define PORT "3457"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold


bool online = true;
struct stack *stack;
int new_fd[BACKLOG];            // UP TO 10 CONNECTIONS
//tbb::mutex mutex;
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
            //tbb::mutex::scoped_lock lock(mutex); /* We implemented mutex using TBB's implementation of scoped lock */
            //pthread_mutex_lock(&mutex);
            numbytes = recv(new_fd, buf, sizeof(buf), 0);
            if (numbytes <=0) {
                perror("recv");
                connected = false;
                break;
            }
            if(push(stack, buf)==false)
            {
                printf("STACK IS FULL!\n");
            }
                    //pthread_mutex_unlock(&mutex);
        } else if (!strcmp(buf, "TOP")) {
            printf("TOP IS AT: %p\n", stack->curraddr);
            if (send(new_fd, top(*stack), 1024, 0) == -1)  {
                perror("send");
            }
           
        } else if (!strcmp(buf, "POP")) {
            //tbb::mutex::scoped_lock lock(mutex); /* We implemented mutex using TBB's implementation of scoped lock */
            //pthread_mutex_lock(&mutex); 
            if (pop(stack)) {
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
    printf("closed a client socket\n");
    close(new_fd);
}

void sig_handler(int signum)
{

    if (signum == SIGINT) {
        printf("program terminating\n");
        online = false;
        for (size_t i = 0; i < BACKLOG; i++)
        {
            close(new_fd[i]);
        }
        
        while (!stack->isEmpty) pop(stack);
        
        close(sockfd);
        printf("program terminated gracefully");
        exit(EXIT_SUCCESS);
    }

}
int main(void)
{
    int fd = open_new_file();
    stack = (struct stack*)mmap(NULL, sizeof(struct stack), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    stack->isEmpty = true;
    stack->size = 0;
    memory_init(stack);

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

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (fork()==0) {
            close(sockfd); // child doesnt need it
            threadfunc(&new_fd[j]);
            return 1;
        }
        j++;
    }
    close(fd);
    return 0;
}
