#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "vector.h"
#include "map.h"

#include "lex.h"
#include "parse.h"
#include "execute.h"

#include "file.h"

void printnode(struct Node* node) {
    if (node == NULL) {
        printf("()");
    }
    else {
        if (node->type == NODE_VAR) {
            printf("(var %s ", node->varname);
            printnode(node->varexpr);
            printf(")");
        }
        else if (node->type == NODE_ASSIGN) {
            printf("(");
            printnode(node->assignnameexpr);
            printf("=");
            printnode(node->assignexpr);
            printf(")");
        }
        else if (node->type == NODE_ARRAY) {
            printf("(array %s[", node->arrayname);
            printnode(node->arraysize);
            printf("])");
        }
        else if (node->type == NODE_IF) {
            printf("(if ");
            printnode(node->condition);
            printf(" {");
            for (int i = 0; i <= node->condbody->last; i++) {
                printnode((struct Node*)node->condbody->body[i]);
            }
            printf("} ");
            printf("else {(");
            for (int j = 0; j <= node->elsebody->last; j++) {
                printnode((struct Node*)node->elsebody->body[j]);
            }
            printf(")})");
        }
        else if (node->type == NODE_WHILE) {
            printf("(while ");
            printnode(node->condition);
            printf(" {");
            for (int i = 0; i <= node->condbody->last; i++) {
                printnode((struct Node*)node->condbody->body[i]);
            }
            printf("})");
        }
        else if (node->type == NODE_BREAK) {
            printf("(break)");
        }
        else if (node->type == NODE_FUNCTION) {
            printf("(function %s<", node->invokablename);
            for (int i = 0; i <= node->invokableparam->last; i++) {
                printf("%s,", node->invokableparam->body[i]);
            }
            printf("> {");
            for (int j = 0; j <= node->invokablebody->last; j++) {
                printnode((struct Node*)node->invokablebody->body[j]);
            }
            printf("})");
        }
        else if (node->type == NODE_RETURN) {
            printf("(return ");
            printnode(node->returnexpr);
            printf(")");
        }
        else if (node->type == NODE_CLASS) {
            printf("(class %s<", node->invokablename);
            for (int i = 0; i <= node->invokableparam->last; i++) {
                printf("%s,", node->invokableparam->body[i]);
            }
            printf("> {");
            for (int j = 0; j <= node->invokablebody->last; j++) {
                printnode((struct Node*)node->invokablebody->body[j]);
            }
            printf("})");
        }
        else if (node->type == NODE_EXPR) {
            printf("<");
            printnode(node->expr);
            printf(">");
        }
        else if (node->type == NODE_NUMBER) {
            printf("%s", node->factor);
        }
        else if (node->type == NODE_STRING) {
            printf("%s", node->factor);
        }
        else if (node->type == NODE_NAME) {
            printf("%s", node->factor);
        }
        else if (node->type == NODE_ATTR) {
            printf("(");
            printnode(node->binaryopleft);
            printf(".");
            printnode(node->binaryopright);
            printf(")");
        }
        else if (node->type == NODE_INDEX) {
            printf("(");
            printnode(node->binaryopleft);
            printf("[");
            printnode(node->binaryopright);
            printf("])");
        }
        else if (node->type == NODE_INVOKE) {
            printf("(");
            printnode(node->invokenameexpr);
            printf("(");
            for (int i = 0; i <= node->invokeparam->last; i++) {
                printnode((struct Node*)node->invokeparam->body[i]);
                printf(",");
            }
            printf("))");
        }
        else if (node->type == NODE_NEG) {
            printf("(-");
            printnode(node->unaryopexpr);
            printf(")");
        }
        else if (node->type == NODE_LENGTH) {
            printf("($");
            printnode(node->unaryopexpr);
            printf(")");
        }
        else if (node->type == NODE_AND) {
            printf("(");
            printnode(node->binaryopleft);
            printf("&&");
            printnode(node->binaryopright);
            printf(")");
        }
        else if (node->type == NODE_NOT) {
            printf("(!");
            printnode(node->unaryopexpr);
            printf(")");
        }
    }
}

void printvalue(struct Value* value) {
    if (value->type == VALUE_NUMBER) {
        printf("%f", value->number);
    }
    else if (value->type == VALUE_STRING) {
        printf("%s", value->string);
    }
    else if (value->type == VALUE_ARRAY) {
        printf("{");
        for (int i = 0; i < value->array->size; i++) {
            printvalue(value->array->body[i]);
            printf(", ");
        }
        printf("}");
    }
    else if (value->type == VALUE_FUNCTION) {
        printf("function %s(", value->invokable->name);
        for (int i = 0; i <= value->invokable->param->last; i++) {
            printf("%s, ", value->invokable->param->body[i]);
        }
        printf(")");
    }
    else if (value->type == VALUE_CLASS) {
        printf("class %s(", value->invokable->name);
        for (int i = 0; i <= value->invokable->param->last; i++) {
            printf("%s, ", value->invokable->param->body[i]);
        }
        printf(")");
    }
    else if (value->type == VALUE_OBJECT) {
        printf("object %s ", value->object->name);
        printf("{");
        struct Map* map = value->object->variables->body[value->object->variables->last];
        for (int i = 0; i <= 126; i++) {
            struct Vector* buckets = map->buckets[i];
            for (int j = 0; j <= buckets->last; j++) {
                struct KeyValuePair* kvp = buckets->body[j];
                struct Value* value = (struct Value*)kvp->value;

                printf("%s : ", kvp->key);
                printvalue(value);
                printf(", ");
            }
        }
        printf("}");
    }
    else {
        printf("what the %d", value->type);
    }
}

int main() {
    /*
    clock_t start = clock();
    
    char* testcode = openfile("test.chr");
    struct Lexer* testlexer = lex(testcode); 

    struct Parser* testparser = parse(testlexer);
    
    struct Object* testobject = malloc(sizeof(struct Object));
    testobject->name = "test";
    testobject->variables = vector();
    push(testobject->variables, map());

    struct Vector* references = vector();
    
    execute(testobject, testparser->result, testparser, references, 0);
    
    struct Value* value = malloc(sizeof(struct Value));
    value->type = VALUE_OBJECT;
    value->object = testobject;

    printvalue(value);
    
    clock_t end = clock();

    printf("\ntook %fs\n", (double)(end - start) / CLOCKS_PER_SEC);
    */
    
    executemain(openfile("test.chr"));
    
    return 0;
}
