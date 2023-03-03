#ifndef __EXECUTE_H__
#define __EXECUTE_H__

enum ValueType {
    VALUE_NUMBER = 0,
    VALUE_STRING,
    VALUE_ARRAY,
    VALUE_FUNCTION,
    VALUE_CLASS,
    VALUE_OBJECT
};

struct Array {
    struct Value** body;
    int size;
};

struct Invokable {
    char* name;
    struct Vector* param;
    struct Vector* body;
    struct Vector* variables;
};

struct Object {
    char* name;
    struct Vector* variables;
};

struct Value {
    union {
        double number;
        char* string;
        struct Array* array;
        struct Invokable* invokable;
        struct Object* object;
    };

    enum ValueType type;
};

struct ExecuteState {
    struct Value* returnvalue;
    int isreturn;
    int isbreak;
};

struct ExecuteState* execute(struct Object* object, struct Vector* body, struct Parser* parser, struct Vector* references, int isloop);

#endif