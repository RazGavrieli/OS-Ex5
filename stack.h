#include <stdbool.h>

struct node {
    char text[1024];
    struct node *prev;
    struct node *next;
};


struct stack {
    struct node *ptr;
    bool isEmpty = true;
};
// supporting functions
bool push(struct stack*, char*);
char* top(struct stack);
bool pop (struct stack*);