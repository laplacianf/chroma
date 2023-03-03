#ifndef __PARSE_H__
#define __PARSE_H__

enum NodeType {
    NODE_NUMBER = 0,
    NODE_STRING,
    NODE_NAME,
    NODE_VAR,
    NODE_ASSIGN,
    NODE_ARRAY,
    NODE_IF,
    NODE_WHILE,
    NODE_BREAK,
    NODE_FUNCTION,
    NODE_RETURN,
    NODE_CLASS,
    NODE_EXPR,
    NODE_INVOKE,
    NODE_INDEX,
    NODE_ATTR,
    NODE_ADD,
    NODE_NEG,
    NODE_MUL,
    NODE_DIV,
    NODE_NOT,
    NODE_AND,
    NODE_EQ,
    NODE_LESS,
    NODE_LENGTH
};

struct Node {
    union {
        //var
        struct {
            char* varname;
            struct Node* varexpr;
        };
        //assign
        struct {
            struct Node* assignnameexpr;
            struct Node* assignexpr;
        };
        //array
        struct {
            char* arrayname;
            struct Node* arraysize;
        };
        //if & while(condition)
        struct {
            struct Node* condition;
            struct Vector* condbody;
            struct Vector* elsebody;
        };
        //function & class(invokable)
        struct {
            char* invokablename;
            struct Vector* invokableparam;
            struct Vector* invokablebody;
        };
        //return
        struct {
            struct Node* returnexpr;
        };
        //function & class invoke
        struct {
            struct Node* invokenameexpr;
            struct Vector* invokeparam;
        };
        //expression
        struct {
            struct Node* expr;
        };
        //unaryop
        struct {
            struct Node* unaryopexpr;
        };
        //binaryop
        struct {
            struct Node* binaryopleft;
            struct Node* binaryopright;
        };
        //factor 
        struct {
            char* factor;
        };
    };

    enum NodeType type;
    int linepos;
};

struct Parser {
    struct Vector* tokens;
    struct Vector* lines;
    struct Vector* result;

    int pos;
    struct Token* previous;
    struct Token* current;
};

struct Parser* parse(struct Lexer* lexer);

#endif
