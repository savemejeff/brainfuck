#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "stack.h"

#define CELLS_CAPACITY 30000

int8_t cells[CELLS_CAPACITY];
int ptr;
char* source;
long length;
int curr;
Stack* stack;

void left()
{
    if (ptr == 0)
    {
        ptr = CELLS_CAPACITY - 1;
    }
    else
    {
        ptr--;
    }
}

void right()
{
    if (ptr == CELLS_CAPACITY - 1)
    {
        ptr = 0;
    }
    else 
    {
        ptr++;
    }
}

void increase()
{
    cells[ptr]++;
}

void decrease()
{
    cells[ptr]--;
}

bool isAtEnd()
{
    return curr >= length;
}

char peek()
{
    if (isAtEnd()) return '\0';
    return source[curr];
}

char advance()
{
    if (isAtEnd()) return '\0';
    return source[curr++];
}

void output()
{
    printf("%c", cells[ptr]);
}

void input()
{
    cells[ptr] = getchar();
}

void loop()
{
    if (cells[ptr] == 0)
    {
        int start = curr;
        pushStack(stack, curr);
        bool found = false;
        while (!found)
        {
            switch (advance())
            {
                case '[':
                    pushStack(stack, curr);
                    break;
                case ']':
                    int top = popStack(stack);
                    found = top == start;
                    break;
                default:
                    break;
            }
        }
    }
    else 
    {
        pushStack(stack, curr);
    }
}

void endLoop()
{
    
    if (cells[ptr] != 0)
    {
        curr = peekStack(stack);
    }
    else
    {
        popStack(stack);
    }
}

void readSource(char* path)
{
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);

    source = (char*)malloc(length + 1);
    fread(source, length, 1, f);
    fclose(f);

    source[length] = 0;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: brainfuck [path]");
        return -1;
    }

    Stack s;
    stack = &s;
    initStack(stack);
    char* path = argv[1];
    curr = 0;
    ptr = 0;

    readSource(path);
    
    while (!isAtEnd())
    {
        switch (advance())
        {
            case '<': left(); break;
            case '>': right(); break;
            case '+': increase(); break;
            case '-': decrease(); break;
            case '.': output(); break;
            case ',': input(); break;
            case '[': loop(); break;
            case ']': endLoop(); break;
            default: break;
        }
    }

    freeStack(stack);
    return 0;
}