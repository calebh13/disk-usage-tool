#include <stdlib.h>
#include <stdio.h>

struct stack {
    void** arr;
    size_t max_size;
    size_t cur_size;
};

void stack_alloc(struct stack* s);
int stack_push(struct stack* s, void* item);
void* stack_pop(struct stack* s);
void* stack_peek(struct stack* s);
int stack_empty(struct stack* s);
