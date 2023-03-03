#include <stdlib.h>

#include "vector.h"

struct Vector* vector() {
    struct Vector* newvector = malloc(sizeof(struct Vector));
    newvector->body = malloc(sizeof(void*));
    newvector->size = 1;
    newvector->last = -1;

    return newvector;
}

static void resize(struct Vector* vector) {
    vector->body = realloc(vector->body, (vector->size * 2) * sizeof(void*));
    vector->size *= 2;
} 

void push(struct Vector* vector, void* value) {
    if (vector->last >= vector->size - 1) {
        resize(vector);
    }

    vector->body[++vector->last] = value;
}
