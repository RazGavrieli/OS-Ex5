#include "stack.h"
#include <stdbool.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>       // EXIT_SUCCESS
#include <stdio.h>        // perror
#include <unistd.h>       // read/write/lseek/STDIN_FILENO
#include <fcntl.h>
#include <string.h>
#include <time.h>

int fd;
struct flock locker; // we will use this to synchronize the operation of the processes

int open_new_file()
{
    fd = open("locker.txt", O_WRONLY | O_CREAT); 
    // O_WRONLY - open for writing only
    //O_CREAT - If the file exists, this flag has no effect
    if (fd == -1)//The file didn't opened successfuly
    {
        printf("Error");
        
    }
    memset(&locker, 0, sizeof(locker)); 
   return fd;
}
char* memory_init(struct stack* stack) {
    char *initialptr = (char*)mmap(NULL, sizeof(struct node)*10, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if ((void*)-1 == initialptr) {
        printf("error could not map memory");
        exit(1);
    }
    stack->curraddr = initialptr;
    printf("initial ptr: %p\n", initialptr);
    return initialptr;
}
void* _malloc (size_t size, struct stack* stack) {
    stack->curraddr += sizeof(struct node);
    return stack->curraddr;
}

void _free (void* ptr, struct stack* stack) {
    stack->curraddr -= sizeof(struct node);
   // return stack->curraddr;
}

bool push(struct stack *stack, char* text) {
    locker.l_type = F_WRLCK;    //write lock
    fcntl(fd, F_SETLKW, &locker); //the f_setlkw request waits until the specified range becomes free and the request can be completed
    struct node *newNode = (struct node*)_malloc(sizeof(struct node), stack);
    strcpy(newNode->text, text);
    if (stack->isEmpty) {
        stack->ptr = newNode;
        stack->isEmpty = false;
    } else {
        newNode->prev = stack->ptr;
        stack->ptr = newNode;
    }
    stack->size++;
    locker.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &locker);
    return true;
}

bool pop(struct stack *stack) {
    locker.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &locker);
    if (stack->isEmpty) {
        return false;
    }
    struct node *tempNode = stack->ptr;
    _free(stack->ptr, stack);
    if (tempNode->prev == NULL) {
        stack->isEmpty = true;
        stack->ptr = NULL;
        return true;
    }
    tempNode->prev->next = NULL;
    stack->ptr = stack->ptr->prev;
    stack->size--;
    locker.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &locker);
    return true;
}
char* top(struct stack stack) {
    locker.l_type = F_WRLCK;
    fcntl(fd, F_SETLKW, &locker);
    if (!stack.ptr) {
        char* emptyStack = "stack is empty!";
        return emptyStack;
    }
    locker.l_type = F_UNLCK;
    fcntl (fd, F_SETLKW, &locker);
    return stack.ptr->text;
}

// int main() {
//     struct stack stack;
//     push(&stack, "1");
//     printf("%s\n", top(stack));
//     push(&stack, "2");
//     printf("%s\n", top(stack));
//     push(&stack, "3");
//     printf("%s\n", top(stack));
//     pop(&stack);
//     printf("%s\n", top(stack));
//     wipe(stack);
// }