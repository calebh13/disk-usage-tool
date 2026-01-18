#include "stack.h"

void stack_alloc(struct stack* s)
{
    s->arr = malloc(sizeof(void*));
    if (s->arr == NULL) {
        fprintf(stderr, "Failure allocating stack\n");
        exit(EXIT_FAILURE);
    }
    s->max_size = 1;
    s->cur_size = 0;
}

int stack_push(struct stack* s, void* item)
{
    if (s->cur_size == s->max_size) {
        void** temp_ptr = (void**)realloc(s->arr, 2 * s->max_size * sizeof(void*));
        if (temp_ptr == NULL) {
            fprintf(stderr, "Ran out of memory");
            free(s->arr);
            return EXIT_FAILURE;
        } else {
            s->arr = temp_ptr;
            s->max_size *= 2;
        }
    }

    s->arr[s->cur_size++] = item;
    return 0;
}

void* stack_pop(struct stack* s)
{
    if (s->cur_size > 0) {
        s->cur_size--;
        return s->arr[s->cur_size];
    }     

    fprintf(stderr, "Tried to pop from empty stack");
    return NULL;
}

int stack_empty(struct stack* s)
{
    return s->cur_size == 0;
}
