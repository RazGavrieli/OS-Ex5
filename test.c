#include <assert.h>
#include "stack.h"
#include <stdio.h>

#include <string.h>


int main() {
    struct stack stack;
    stack.isEmpty = true;
    stack.size = 0;
    char* test1 = "test";
    char* test2 = "22222";
    push(&stack, test1);
    char* test1res = top(stack);
    push(&stack, test2);
    char* test2res = top(stack);
    pop(&stack);
    char* test1POPres = top(stack);
    
    
    assert (!strncmp(test1, test1res, 4));
    assert (!strncmp(test2, test2res, 4));
    assert (strncmp(test1, test2res, 4));
    assert (strncmp(test2, test1res, 4));
    assert (!strncmp(test1, test1POPres, 4));
    
    while (!stack.isEmpty) pop (&stack);
    /* STACK IS NOW EMPTY */
    for (int i = 0; i < 10; i++)
    {
        char num = i+'0';
        char* finalnum = &num;
        push(&stack, finalnum);
    }
    for (int i = 0; i < 10; i++)
    {
        char* currNum = top(stack);
        pop(&stack);
        char num = (9-i)+'0';
        char* finalnum = &num;
        assert (!strncmp(currNum, finalnum, 1));
    }
    
    

    
    
    //assert(strcmp(top(stack), test1));
    //assert(0);
    assert(1);
}