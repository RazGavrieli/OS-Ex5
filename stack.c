#include "stack.h"
#include <stdbool.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
int pos = sizeof(struct node); // initial position for the most bottom value in the stack

void* _malloc (size_t size) {
    void* ptr = sbrk(pos);
    pos += sizeof(struct node);
    return ptr;
}

void _free (void* ptr) {
    sbrk(pos);
    pos -= sizeof(struct node);
}



bool push(struct stack *stack, char* text) {
    struct node *newNode = (struct node*)_malloc(sizeof(struct node));
    strcpy(newNode->text, text);
    if (stack->isEmpty) {
        stack->ptr = newNode;
        stack->isEmpty = false;
    } else {
        newNode->prev = stack->ptr;
        stack->ptr = newNode;
    }
    
    return true;
}

bool pop(struct stack *stack) {
    if (stack->isEmpty) {
        return false;
    }
    struct node *tempNode = stack->ptr;
    _free(stack->ptr);
    if (tempNode->prev == NULL) {
        stack->isEmpty = true;
        stack->ptr = NULL;
        return true;
    }
    tempNode->prev->next = NULL;
    stack->ptr = stack->ptr->prev;
    

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