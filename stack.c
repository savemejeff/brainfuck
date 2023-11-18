#include "stack.h"
#include <stdlib.h>

void initStack(Stack* stack)
{
    stack->capacity = 0;
    stack->top = 0;
    stack->array = NULL;
}

void freeStack(Stack* stack)
{
    free(stack->array);
    initStack(stack);
}

void pushStack(Stack* stack, int n)
{
    if (stack->top == stack->capacity)
    {
        stack->capacity = stack->capacity == 0 ? 8 : stack->capacity * 2;
        stack->array = (int*)realloc(stack->array, stack->capacity * sizeof(int));
    }

    stack->array[stack->top++] = n;
}

int popStack(Stack* stack)
{
    return stack->array[--stack->top];
}

int peekStack(Stack* stack)
{
    return stack->array[stack->top - 1];
}