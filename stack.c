#include "stack.h"
#include <stdbool.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>

bool push(struct stack *stack, char* text) {
    struct node *newNode = (struct node*)malloc(sizeof(struct node));
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
    free(stack->ptr);
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