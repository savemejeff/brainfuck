typedef struct
{
    int capacity;
    int top;
    int* array;
} Stack;

void initStack(Stack* stack);
void freeStack(Stack* stack);

void pushStack(Stack* stack, int n);
int popStack(Stack* stack);
int peekStack(Stack* stack);