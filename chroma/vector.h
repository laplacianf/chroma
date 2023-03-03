#ifndef __VECTOR_H__
#define __VECTOR_H__

struct Vector {
    void** body;
    int size;
    int last;
};

struct Vector* vector();
void push(struct Vector* vector, void* value);

#endif