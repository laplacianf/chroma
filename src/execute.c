#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vector.h"
#include "map.h"
#include "lex.h"
#include "parse.h"

#include "execute.h"

#define isdoubleint(d) d == floor(d)
#define char2str(c) ((char[]){(c), (0)})

#define safefree(p)\
    free(p);\
    p = NULL

//add executemain function & freevalue function ==> end!

static char* valuetype2str(enum ValueType type) {
    if (type == VALUE_NUMBER) {
        return "number";
    }
    else if (type == VALUE_STRING) {
        return "string";
    }
    else if (type == VALUE_ARRAY) {
        return "array";
    }
    else if (type == VALUE_FUNCTION) {
        return "function";
    }
    else if (type == VALUE_CLASS) {
        return "class";
    }
    else {
        return "object";
    }
}

static struct Value* getattribute(struct Object* object, char* name) {
    return get((struct Map*)object->variables->body[object->variables->last], name);
}

static struct Value* getvariable(struct Object* object, char* name) {
    struct Value* value = NULL;
    
    for (int scope = object->variables->last; scope >= 0; scope--) {
        value = (struct Value*)get((struct Map*)object->variables->body[scope], name);
        if (value != NULL) break;
    }

    return value;
}

static struct Value* executeexpr(struct Object* object, struct Parser* parser, struct Node* expr, struct Vector* references) {
    struct Value* value;

    if (expr->type == NODE_NUMBER) {
        value = malloc(sizeof(struct Value));
        value->type = VALUE_NUMBER;
        value->number = atof(expr->factor);

        push(references, value);
    }
    else if (expr->type == NODE_STRING) {
        value = malloc(sizeof(struct Value));
        value->type = VALUE_STRING;
        value->string = malloc((strlen(expr->factor) + 1) * sizeof(char));
        strcpy(value->string, expr->factor);

        push(references, value);
    }
    else if (expr->type == NODE_NAME) {
        value = getvariable(object, expr->factor);
        if (value == NULL) {
            printf("not exist! %s", expr->factor);
        }
    } 
    else if (expr->type == NODE_INVOKE) {
        struct Value* invokablevalue = executeexpr(object, parser, expr->invokenameexpr, references);

        if (invokablevalue->type == VALUE_FUNCTION) {
            if (invokablevalue->invokable->param->last == expr->invokeparam->last) {
                struct Object* newobject = malloc(sizeof(struct Object));
                newobject->name = invokablevalue->invokable->name;
                newobject->variables = vector();

                for (int s = 0; s <= invokablevalue->invokable->variables->last; s++) {
                    push(newobject->variables, invokablevalue->invokable->variables->body[s]);
                }

                push(newobject->variables, map());

                struct Vector* param = vector();

                for (int p = 0; p <= expr->invokeparam->last; p++) {
                    set((struct Map*)newobject->variables->body[newobject->variables->last], invokablevalue->invokable->param->body[p], executeexpr(object, parser, expr->invokeparam->body[p], references));
                }

                value = execute(newobject, invokablevalue->invokable->body, parser, references, 0)->returnvalue;
            }
            else {

            }
        }
        else if (invokablevalue->type == VALUE_CLASS) {
            if (invokablevalue->invokable->param->last == expr->invokeparam->last) {
                struct Object* newobject = malloc(sizeof(struct Object));
                newobject->name = invokablevalue->invokable->name;
                newobject->variables = vector();

                for (int s = 0; s <= invokablevalue->invokable->variables->last; s++) {
                    push(newobject->variables, invokablevalue->invokable->variables->body[s]);
                }

                push(newobject->variables, map());

                struct Vector* param = vector();

                for (int p = 0; p <= expr->invokeparam->last; p++) {
                    set((struct Map*)newobject->variables->body[newobject->variables->last], invokablevalue->invokable->param->body[p], executeexpr(object, parser, expr->invokeparam->body[p], references));
                }

                execute(newobject, invokablevalue->invokable->body, parser, references, 0);
                
                value = malloc(sizeof(struct Value));
                value->type = VALUE_OBJECT;
                value->object = newobject;

                push(references, value);
            }
            else {

            }
        }
        else {
            printf("At line %d:\n\t%s\nType \'%s\' not invokable\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(invokablevalue->type));
            exit(1);
        }
    }
    else if (expr->type == NODE_INDEX) {
        struct Value* left = executeexpr(object, parser, expr->binaryopleft, references);
        struct Value* right = executeexpr(object, parser, expr->binaryopright, references);

        if (left->type == VALUE_STRING) {
            if (right->type == VALUE_NUMBER) {
                if (right->number < strlen(left->string)) {
                    value = malloc(sizeof(struct Value));
                    value->type = VALUE_NUMBER;
                    value->number = left->string[(int)round(right->number)];

                    push(references, value);
                }
                else {
                    printf("At line %d:\n\t%s\n\'string\' index out of range\n", expr->linepos + 1, parser->lines->body[expr->linepos]);
                    exit(1);
                }
            }
            else {
                printf("At line %d:\n\t%s\nIndex must be type \'number\' not type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(right->type));
                exit(1);
            }
        }
        else if (left->type == VALUE_ARRAY) {
            if (right->type == VALUE_NUMBER) {
                if (right->number < left->array->size) {
                    value = left->array->body[(int)round(right->number)];
                }
                else {
                    printf("At line %d:\n\t%s\n\'array\' index out of range\n", expr->linepos + 1, parser->lines->body[expr->linepos]);
                    exit(1);
                }
            }
            else {
                printf("At line %d:\n\t%s\nIndex must be type \'number\' not type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(right->type));
                exit(1);
            }
        }
        else {
            printf("At line %d:\n\t%s\nType \'%s\' not indexable\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(left->type));
            exit(1);
        } 
    }
    else if (expr->type == NODE_ATTR) {
        
    }
    else if (expr->type == NODE_ADD) {
        struct Value* left = executeexpr(object, parser, expr->binaryopleft, references);
        struct Value* right = executeexpr(object, parser, expr->binaryopright, references);

        if (left->type == VALUE_NUMBER) {
            if (right->type == VALUE_NUMBER) {
                value = malloc(sizeof(struct Value));
                value->type = VALUE_NUMBER;
                value->number = left->number + right->number;

                push(references, value);
            }
            else if (right->type == VALUE_STRING) {
                value = malloc(sizeof(struct Value));
                value->type = VALUE_STRING;
                value->string = malloc(2 * sizeof(char));
                strcpy(value->string, char2str(left->number));
                value->string = realloc(value->string, (strlen(right->string) + 2) * sizeof(char));
                strcat(value->string, right->string);

                push(references, value);
            }
            else {
                printf("At line %d:\n\t%s\nUnsupported operation \'+\' between type \'number\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(right->type));
                exit(1);
            }
        }
        else if (left->type == VALUE_STRING) {
            if (right->type == VALUE_NUMBER) {
                value = malloc(sizeof(struct Value));
                value->type = VALUE_STRING;
                value->string = malloc((strlen(left->string) + 1) * sizeof(char));
                strcpy(value->string, left->string);
                value->string = realloc(value->string, (strlen(left->string) + 2) * sizeof(char));
                strcat(value->string, char2str(right->number));

                push(references, value);
            }
            else if (right->type == VALUE_STRING) {
                value = malloc(sizeof(struct Value));
                value->type = VALUE_STRING;
                value->string = malloc((strlen(left->string) + 1) * sizeof(char));
                strcpy(value->string, left->string);
                value->string = realloc(value->string, (strlen(left->string) + strlen(right->string) + 1) * sizeof(char));
                strcat(value->string, right->string);

                push(references, value);
            }
            else {
                printf("At line %d:\n\t%s\nUnsupported operation \'+\' between type \'string\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(right->type));
                exit(1);
            }
        }
        else {
            printf("At line %d:\n\t%s\nUnsupported operation \'+\' between type \'%s\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(left->type), valuetype2str(right->type));
            exit(1);
        }
    }
    else if (expr->type == NODE_NEG) {
        struct Value* nodevalue = executeexpr(object, parser, expr->unaryopexpr, references);

        if (nodevalue->type == VALUE_NUMBER) {
            value = malloc(sizeof(struct Value));
            value->type = VALUE_NUMBER;
            value->number = -(nodevalue->number);

            push(references, value);
        }
        else {
            printf("At line %d:\n\t%s\nUnsupported operation \'-\' for type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(nodevalue->type));
            exit(1);
        }
    }
    else if (expr->type == NODE_MUL) {
        struct Value* left = executeexpr(object, parser, expr->binaryopleft, references);
        struct Value* right = executeexpr(object, parser, expr->binaryopright, references);

        if (left->type == VALUE_NUMBER) {
            if (right->type == VALUE_NUMBER) {
                value = malloc(sizeof(struct Value));
                value->type = VALUE_NUMBER;
                value->number = left->number * right->number;

                push(references, value);
            }
            else {
                printf("At line %d:\n\t%s\nUnsupported operation \'*\' between type \'number\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(right->type));
                exit(1);
            }
        }
        else {
            printf("At line %d:\n\t%s\nUnsupported operation \'*\' between type \'%s\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(left->type), valuetype2str(right->type));
            exit(1);
        }
    }
    else if (expr->type == NODE_DIV) {
        struct Value* left = executeexpr(object, parser, expr->binaryopleft, references);
        struct Value* right = executeexpr(object, parser, expr->binaryopright, references);
        
        if (left->type == VALUE_NUMBER) {
            if (right->type == VALUE_NUMBER) { 
                if (right->number != 0) {
                    value = malloc(sizeof(struct Value));
                    value->type = VALUE_NUMBER;
                    value->number = left->number / right->number;

                    push(references, value);
                }
                else {
                    printf("At line %d:\n\t%s\nCannot divide by \'0\'", expr->linepos + 1, parser->lines->body[expr->linepos]);
                    exit(1);
                }
            }
            else {
                printf("At line %d:\n\t%s\nUnsupported operation \'/\' between type \'number\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(right->type));
                exit(1);
            }
        }
        else {
            printf("At line %d:\n\t%s\nUnsupported operation \'/\' between type \'%s\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(left->type), valuetype2str(right->type));
            exit(1);
        }
    }
    else if (expr->type == NODE_NOT) {
        struct Value* nodevalue = executeexpr(object, parser, expr->unaryopexpr, references);
        if (nodevalue->type == VALUE_NUMBER) {
            value = malloc(sizeof(struct Value));
            value->type = VALUE_NUMBER;
            value->number = !(nodevalue
                              ->number);

            push(references, value);
        }
        else {
            printf("At line %d:\n\t%s\nUnsupported operation \'!\' for type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(nodevalue->type));
            exit(1);
        }
    }
    else if (expr->type == NODE_AND) {
        struct Value* left = executeexpr(object, parser, expr->binaryopleft, references);
        struct Value* right = executeexpr(object, parser, expr->binaryopright, references);

        if (left->type == VALUE_NUMBER) {
            if (right->type == VALUE_NUMBER) {
                value = malloc(sizeof(struct Value));
                value->type = VALUE_NUMBER;
                value->number = left->number && right->number;

                push(references, value);
            }
            else {
                printf("At line %d:\n\t%s\nUnsupported operation \'&&\' between type \'number\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(right->type));
                exit(1);
            }
        }
        else {
            printf("At line %d:\n\t%s\nUnsupported operation \'&&\' between type \'%s\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(left->type), valuetype2str(right->type));
            exit(1);
        }
    }
    else if (expr->type == NODE_EQ) {
        struct Value* left = executeexpr(object, parser, expr->binaryopleft, references);
        struct Value* right = executeexpr(object, parser, expr->binaryopright, references);
        
        if (left->type != right->type) {
            printf("At line %d:\n\t%s\nUnsupported operation \'==\' between type \'%s\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(left->type), valuetype2str(right->type));
            exit(1);
        }
        else {
            value = malloc(sizeof(struct Value));
            value->type = VALUE_NUMBER;
            
            if (left->type == VALUE_NUMBER) {
                value->number = left->number == right->number;
            }
            else if (left->type == VALUE_STRING) {
                value->number = !strcmp(left->string, right->string);
            }
            else if (left->type == VALUE_ARRAY) {
                value->number = left->array == right->array;
            }
            else if (left->type == VALUE_FUNCTION || left->type == VALUE_CLASS) {
                value->number = left->invokable == right->invokable;
            }
            else if (left->type == VALUE_OBJECT) {
                value->number = left->object == right->object;
            }
            
            push(references, value);
            
        }
    }
    else if (expr->type == NODE_LESS) {
        struct Value* left = executeexpr(object, parser, expr->binaryopleft, references);
        struct Value* right = executeexpr(object, parser, expr->binaryopright, references);

        if (left->type == VALUE_NUMBER) {
            if (right->type == VALUE_NUMBER) {
                value = malloc(sizeof(struct Value));
                value->type = VALUE_NUMBER;
                value->number = left->number < right->number;

                push(references, value);
            }
            else {
                printf("At line %d:\n\t%s\nUnsupported operation \'<\' between type \'number\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(right->type));
                exit(1);
            }
        }
        else {
            printf("At line %d:\n\t%s\nUnsupported operation \'<\' between type \'%s\' and type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(left->type), valuetype2str(right->type));
            exit(1);
        }
    }
    else if (expr->type == NODE_LENGTH) {
        struct Value* nodevalue = executeexpr(object, parser, expr->unaryopexpr, references);

        if (nodevalue->type == VALUE_STRING) {
            value = malloc(sizeof(struct Value));
            value->type = VALUE_NUMBER;
            value->number = strlen(nodevalue->string);

            push(references, value);
        }
        else if (nodevalue->type == VALUE_ARRAY) {
            value = malloc(sizeof(struct Value));
            value->type = VALUE_NUMBER;
            value->number = nodevalue->array->size;

            push(references, value);
        }
        else {
            printf("At line %d:\n\t%s\nUnsupported operation \'$\' for type \'%s\'\n", expr->linepos + 1, parser->lines->body[expr->linepos], valuetype2str(nodevalue->type));
            exit(1);
        }
    }
 
    return value;
} 

struct ExecuteState* execute(struct Object* object, struct Vector* body, struct Parser* parser, struct Vector* references, int isloop) {
    struct ExecuteState* state = malloc(sizeof(struct ExecuteState));
    struct Value* zero = malloc(sizeof(struct Value));
    zero->type = VALUE_NUMBER;
    zero->number = 0;

    push(references, zero);

    state->returnvalue = zero;
    state->isreturn = 0;
    state->isbreak = 0;
    
    for (int pos = 0; pos <= body->last; pos++) {
        struct Node* current = (struct Node*)body->body[pos];

        if (current->type == NODE_VAR) {
            if (current->varexpr != NULL) {
                set((struct Map*)object->variables->body[object->variables->last], current->varname, executeexpr(object, parser, current->varexpr, references));
            }
            else {
                struct Value* zero = malloc(sizeof(struct Value));
                zero->type = VALUE_NUMBER;
                zero->number = 0;

                push(references, zero);

                set((struct Map*)object->variables->body[object->variables->last], current->varname, zero);
            }
        }
        else if (current->type == NODE_ASSIGN) {
            if (current->assignnameexpr->type == NODE_NAME) {
                int isfound = 0;

                for (int scope = object->variables->last; scope >= 0; scope--) {
                    struct Value* value = get((struct Map*)object->variables->body[scope], current->assignnameexpr->factor);
                    if (value != NULL) {
                        set((struct Map*)object->variables->body[scope], current->assignnameexpr->factor, executeexpr(object, parser, current->assignexpr, references));
                        isfound = 1;
                    }
                }

                if (!isfound) {
                    printf("At line %d:\n\t%s\nUndefined variable: \'%s\'\n", current->linepos + 1, parser->lines->body[current->linepos], current->assignnameexpr->factor);
                    exit(1);
                }
            }
            else if (current->assignnameexpr->type == NODE_ATTR) {

            }
            else if (current->assignnameexpr->type == NODE_INDEX) {

            }
        }
        else if (current->type == NODE_ARRAY) {
            struct Value* sizevalue = executeexpr(object, parser, current->arraysize, references);

            if (sizevalue->type == VALUE_NUMBER) {
                int size = (int)round(sizevalue->number);
                struct Array* array = malloc(sizeof(struct Array));
                array->size = size;
                array->body = malloc(size * sizeof(struct Value*));

                for (int i = 0; i < size; i++) {
                    zero->type = VALUE_NUMBER;
                    zero->number = 0;

                    push(references, zero);

                    array->body[i] = zero;
                }

                struct Value* value = malloc(sizeof(struct Value));
                value->type = VALUE_ARRAY;
                value->array = array;

                push(references, value);

                set((struct Map*)object->variables->body[object->variables->last], current->arrayname, value);
            }
            else {
                printf("At line %d:\n\t%s\nIndex must be type \'number\' not type \'%s\'\n", current->linepos + 1, parser->lines->body[current->linepos], valuetype2str(sizevalue->type));
                exit(1);
            }
        }
        else if (current->type == NODE_IF) {
            struct Value* condition = executeexpr(object, parser, current->condition, references);
            struct ExecuteState* ifstate;

            if (condition->type != VALUE_NUMBER) {
                ifstate = execute(object, current->condbody, parser, references, isloop);
            }
            else if (condition->number != 0) {
                ifstate = execute(object, current->condbody, parser, references, isloop);
            }
            else {
                ifstate = execute(object, current->elsebody, parser, references, isloop);
            }

            if (ifstate->isreturn || (isloop && ifstate->isbreak)) {
                state = ifstate;
                break;
            }
        }
        else if (current->type == NODE_WHILE) {
            struct ExecuteState* whilestate;
            int isreturn = 0;

            for(;;) {
                struct Value* condition = executeexpr(object, parser, current->condition, references);

                if (condition->type != VALUE_NUMBER) {
                    whilestate = execute(object, current->condbody, parser, references, 1);
                }
                else if (condition->number != 0) {
                    whilestate = execute(object, current->condbody, parser, references, 1);
                }
                else {
                    break;
                }

                if (whilestate->isreturn) {
                    isreturn = 1;
                    break;
                }
                if (whilestate->isbreak) {
                    break;
                }
            }

            if (isreturn) {
                state = whilestate;
                break;
            }
        }
        else if (current->type == NODE_BREAK) {
            state->isbreak = 1;
            break;
        }
        else if (current->type == NODE_FUNCTION) {
            struct Invokable* function = malloc(sizeof(struct Invokable));
            function->name = current->invokablename;
            function->param = current->invokableparam;
            function->body = current->invokablebody;
            function->variables = vector();

            for (int s = 0; s <= object->variables->last; s++) {
                push(function->variables, object->variables->body[s]);
            }

            struct Value* value = malloc(sizeof(struct Value));
            value->type = VALUE_FUNCTION;
            value->invokable = function;

            push(references, value);

            set((struct Map*)object->variables->body[object->variables->last], current->invokablename, value);
        }
        else if (current->type == NODE_RETURN) {
            state->returnvalue = executeexpr(object, parser, current->returnexpr, references);
            state->isreturn = 1;
            break;
        }
        else if (current->type == NODE_CLASS) {
            struct Invokable* class = malloc(sizeof(struct Invokable));
            class->name = current->invokablename;
            class->param = current->invokableparam;
            class->body = current->invokablebody;
            class->variables = vector();

            for (int s = 0; s <= object->variables->last; s++) {
                push(class->variables, object->variables->body[s]);
            }

            struct Value* value = malloc(sizeof(struct Value));
            value->type = VALUE_CLASS;
            value->invokable = class;

            push(references, value);

            set((struct Map*)object->variables->body[object->variables->last], current->invokablename, value);
        }
        else if (current->type == NODE_EXPR) {
            executeexpr(object, parser, current->expr, references);
        }
    }

    return state;
}


static void freenode(struct Node* node);
static void freebody(struct Vector* bodyvector) {
    for (int i = 0; i <= bodyvector->last; i++) {
        freenode(bodyvector->body[i]);
    }
    safefree(bodyvector->body);
    safefree(bodyvector);
}


static void freenode(struct Node* node) {
    if (node->type == NODE_VAR) {
        safefree(node->varname);
        freenode(node->varexpr);
    }
    else if (node->type == NODE_ASSIGN) {
        freenode(node->assignnameexpr);
        freenode(node->assignexpr);
    }
    else if (node->type == NODE_ARRAY) {
        safefree(node->arrayname);
        freenode(node->arraysize);
    }
    else if (node->type == NODE_IF || node->type == NODE_WHILE) {
        freenode(node->condition);
        freebody(node->condbody);
        freebody(node->elsebody);
    }
    else if (node->type == NODE_FUNCTION || node->type == NODE_CLASS) {
        safefree(node->invokablename);
        
        for (int i = 0; i <= node->invokableparam->last; i++) {
            safefree(node->invokableparam->body[i]);
        }
        safefree(node->invokableparam->body);
        safefree(node->invokableparam);
        
        freebody(node->invokablebody);
    }
    else if (node->type == NODE_RETURN) {
        freenode(node->returnexpr);
    }
    else if (node->type == NODE_INVOKE) {
        freenode(node->invokenameexpr);
        freebody(node->invokeparam);
    }
    else if (node->type == NODE_EXPR) {
        freenode(node->expr);
    }
    else if (node->type == NODE_NEG || node->type == NODE_LENGTH) {
        freenode(node->unaryopexpr);
    }
    else if (node->type == NODE_NUMBER || node->type == NODE_STRING || node->type == NODE_NAME) {
        safefree(node->factor);
    }
    else {
        freenode(node->binaryopleft);
        freenode(node->binaryopright);
    }
    
    safefree(node);
}

void freevalue(struct Value* value) {
    if (value->type == VALUE_STRING) {
        safefree(value->string);
    }
    else if (value->type == VALUE_ARRAY) {
        
    }
    else if (value->type == VALUE_FUNCTION || value->type == VALUE_CLASS) {
        safefree(value->invokable->name);
        
        for (int i = 0; i <= value->invokable->param->last; i++) {
            safefree(value->invokable->param->body[i]);
        }
        safefree(value->invokable->param->body);
        safefree(value->invokable->param);
        
        freebody(value->invokable->body);
        
        for (int i = 0; i <= value->invokable->variables->last; i++) {
            struct Map* variablemap = value->invokable->variables->body[i];
            for (int j = 0; j < MAP_SIZE; j++) {
                struct Vector* bucket = variablemap->buckets[j];
                
                for (int k = 0; k <= bucket->last; k++) {
                    freevalue(bucket->body[k]);
                }
                
                safefree(bucket->body);
                safefree(bucket);
            }
            safefree(variablemap->buckets);
            safefree(variablemap);
        }
        
        safefree(value->invokable->variables->body);
        safefree(value->invokable->variables);
    }
    else if (value->type == VALUE_OBJECT) {
        safefree(value->object->name);
        for (int i = 0; i <= value->object->variables->last; i++) {
            struct Map* variablemap = value->object->variables->body[i];
            for (int j = 0; j < MAP_SIZE; j++) {
                struct Vector* bucket = variablemap->buckets[j];
                
                for (int k = 0; k <= bucket->last; k++) {
                    freevalue(bucket->body[k]);
                }
                
                safefree(bucket->body);
                safefree(bucket);
            }
            safefree(variablemap->buckets);
            safefree(variablemap);
        }
        safefree(value->object->variables->body);
        safefree(value->object->variables);
    }
    
    safefree(value);
}


void executemain(char* code) {
    struct Lexer* mainlexer = lex(code);
    struct Parser* mainparser = parse(mainlexer);
    
    struct Object* mainobject = malloc(sizeof(struct Object));
    mainobject->name = NULL;
    mainobject->variables = vector();
    push(mainobject->variables, map());

    struct Vector* references = vector();
    
    execute(mainobject, mainparser->result, mainparser, references, 0);
    
    for (int i = 0; i <= references->last; i++) {
        freevalue(references->body[i]);
    }
    
    struct Value* mainobjectvalue = malloc(sizeof(struct Value));
    mainobjectvalue->object = mainobject;
    mainobjectvalue->type = VALUE_OBJECT;
    
    freevalue(mainobjectvalue);
    
    //free parser
}
