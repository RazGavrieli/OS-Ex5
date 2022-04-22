/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <iostream>
#include <string>
using namespace std;


#define PORT "3491" // the port client will be connecting to 

#define MAXDATASIZE 1024 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    std::cout << "test";
    fprintf(stdin,"test2\n");
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    printf("here");
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    /*
        The following while true loop listens to the client's input.
        For each correct input it communicates with the server over TCP.
    */
   bool connected = true;
   while (connected) {
        string command; 
        cin >> command;
        if (command == "PUSH") {
            /*
                Here we send to the server PUSH to command it to get ready to our text
                that we want to push. Then, when the server is ready, we send it the text itself.
            */
            if (send(sockfd, "PUSH", 1024, 0) == -1)  {
                perror("send");
            }
            string line;
            getline(cin, line);
            char* c = const_cast<char*>(line.c_str());
            
            if (send(sockfd, c, 1024, 0) == -1)  {
                perror("send");
            }
        } else if (command == "POP") {
            /*
                Here we send to the server 'POP' to command it to pop the stack. 
            */
            if (send(sockfd, "POP", 1024, 0) == -1)  {
                perror("send");
            }
            numbytes = recv(sockfd, buf, sizeof(buf), 0);
            if (numbytes <=0) {
                perror("recv");
                exit(1);
            }
            printf("%s\n", buf);
        } else if (command == "TOP") {
            /*
                Here we send to the sever 'TOP', to command it to send us back the top value.
                Then we receive from the server a message with the top value, or a message 
                saying that the stack is empty. 
            */
            if (send(sockfd, "TOP", 1024, 0) == -1)  {
                perror("send");
            }
            numbytes = recv(sockfd, buf, sizeof(buf), 0);
            if (numbytes <=0) {
                perror("recv");
                exit(1);
            }
            printf("OUTPUT: %s\n", buf);
        } else if (command == "EXIT") {
            connected = false;
        }
   }
    close(sockfd);

    return 0;
}