#include "stack.h"
#include <stdbool.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/mman.h>

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
    return true;
}

bool pop(struct stack *stack) {
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

    return true;
}
char* top(struct stack stack) {
    if (!stack.ptr) {
        char* emptyStack = "stack is empty!";
        return emptyStack;
    }
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