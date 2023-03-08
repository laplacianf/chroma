#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"
#include "lex.h"

#include "parse.h"

#define safefree(p)\
    free(p);\
    p = NULL

static void advance(struct Parser* parser) {
    parser->previous = parser->current;
    ++parser->pos;
    if (parser->pos > parser->tokens->last) {
        parser->current = NULL;
    }
    else {
        parser->current = (struct Token*)parser->tokens->body[parser->pos];
    }
}

static struct Node* parseexpr(struct Parser* parser); //to prevent implicit defintion


static struct Node* parsevalue(struct Parser* parser) {
    struct Node* result = malloc(sizeof(struct Node));

    if (parser->current == NULL) {
        printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos], parser->previous->value);
        exit(1);
    }
    else {
        if (parser->current->type == TOK_NUMBER) {
            result->type = NODE_NUMBER;
            result->linepos = parser->current->linepos;
            result->factor = malloc((strlen(parser->current->value) + 1) * sizeof(char));
            strcpy(result->factor, parser->current->value);

            advance(parser);
        }
        else if (parser->current->type == TOK_STRING) {
            result->type = NODE_STRING;
            result->linepos = parser->current->linepos;
            result->factor = malloc((strlen(parser->current->value) + 1) * sizeof(char));
            strcpy(result->factor, parser->current->value);

            advance(parser);
        }
        else if (parser->current->type == TOK_NAME) {
            result->type = NODE_NAME;
            result->linepos = parser->current->linepos;
            result->factor = malloc((strlen(parser->current->value) + 1) * sizeof(char));
            strcpy(result->factor, parser->current->value);

            advance(parser);
        }
        else if (parser->current->type == TOK_NOT) {
            result->type = NODE_NOT;
            result->linepos = parser->current->linepos;
            advance(parser);
            result->unaryopexpr = parsevalue(parser);
        }
        else if (parser->current->type == TOK_PLUS) {
            advance(parser);
            result = parsevalue(parser);
        }
        else if (parser->current->type == TOK_MINUS) {
            result->type = NODE_NEG;
            result->linepos = parser->current->linepos;
            advance(parser);
            result->unaryopexpr = parsevalue(parser);
        }
        else if (parser->current->type == TOK_LENGTH) {
            result->type = NODE_LENGTH;
            result->linepos = parser->current->linepos;
            advance(parser);
            result->unaryopexpr = parsevalue(parser);
        }
        else if (parser->current->type == TOK_LPAREN) {
            advance(parser);
            if (parser->current == NULL) {
                printf("At line %d:\n\t%s\nUnclosed \'(\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                exit(1);
            }
            else {
                if (parser->current->type == TOK_RPAREN) { //empty paren
                    printf("At line %d:\n\t%s\nInvalid syntax: \')\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                    exit(1);
                }   
                else {
                    result = parseexpr(parser);

                    if (parser->current == NULL) {
                        printf("At line %d:\n\t%s\nUnclosed \'(\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                        exit(1);
                    }
                    else {
                        if (parser->current->type != TOK_RPAREN) {
                            printf("At line %d:\n\t%s\nUnclosed \'(\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                            exit(1);
                        }
                        else {
                            advance(parser);
                        }
                    }
                }
            }
        }
        else {
            printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos], parser->current->value);
            exit(1);
        }
    }

    return result;
}

static struct Node* parsecomplexop(struct Parser* parser) {
    struct Node* result = parsevalue(parser);

    while (parser->current != NULL && (parser->current->type == TOK_LINDEX || parser->current->type == TOK_LPAREN || parser->current->type == TOK_ATTR)) {
        if (parser->current->type == TOK_LINDEX) {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_INDEX;
            newnode->linepos = parser->current->linepos;
            newnode->binaryopleft = result;
            advance(parser);
            newnode->binaryopright = parseexpr(parser);

            if (parser->current == NULL) {
                printf("At line %d:\n\t%s\nUnclosed \'[\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                exit(1);
            }
            else {
                if (parser->current->type != TOK_RINDEX) {
                    printf("At line %d:\n\t%s\nUnclosed \'[\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                    exit(1);
                }
                else {
                    advance(parser);
                    result = newnode;
                }
            }
        }
        else if (parser->current->type == TOK_LPAREN) {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_INVOKE;
            newnode->linepos = parser->current->linepos;
            newnode->invokenameexpr = result;
            newnode->invokeparam = vector();
            
            for(;;) {
                advance(parser);
                if (parser->current == NULL) {
                    printf("At line %d:\n\t%s\nUnclosed \'(\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                    exit(1);
                }
                else {
                    if (parser->current->type == TOK_RPAREN) { //empty parameter 
                        break;
                    }

                    push(newnode->invokeparam, parseexpr(parser));

                    if (parser->current == NULL) {
                        printf("At line %d:\n\t%s\nUnclosed \'(\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                        exit(1);
                    }
                    else {
                        if (parser->current->type == TOK_RPAREN) {
                            break;
                        }
                        if (parser->current->type != TOK_COMMA) {
                            printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos], parser->current->value);
                            exit(1);
                        }
                    }
                }
            }

            advance(parser);

            result = newnode;
        }
        else {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_ATTR;
            newnode->linepos = parser->current->linepos;
            newnode->binaryopleft = result;
            advance(parser);
            struct Node* left = parsevalue(parser);

            if (left->type != NODE_NAME) {
                printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos], parser->previous->value);
                exit(1);
            }
            else { 
                newnode->binaryopright = left;
                result = newnode;
            }
        }
    }

    return result; 
}

static struct Node* parsehigharithmeticop(struct Parser* parser) {
    struct Node* result = parsecomplexop(parser);

    while (parser->current != NULL && (parser->current->type == TOK_MUL || parser->current->type == TOK_DIV)) {
        if (parser->current->type == TOK_MUL) {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_MUL;
            newnode->linepos = parser->current->linepos;
            newnode->binaryopleft = result;
            advance(parser);
            newnode->binaryopright = parsecomplexop(parser);

            result = newnode;
        }
        else {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_DIV;
            newnode->linepos = parser->current->linepos;
            newnode->binaryopleft = result;
            advance(parser);
            newnode->binaryopright = parsecomplexop(parser);

            result = newnode;
        }
    }

    return result;
}

static struct Node* parselowarithmeticop(struct Parser* parser) {
    struct Node* result = parsehigharithmeticop(parser);

    while (parser->current != NULL && (parser->current->type == TOK_PLUS || parser->current->type == TOK_MINUS)) {
        if (parser->current->type == TOK_PLUS) {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_ADD;
            newnode->linepos = parser->current->linepos;
            newnode->binaryopleft = result;
            advance(parser);
            newnode->binaryopright = parsehigharithmeticop(parser);

            result = newnode;
        }
        else {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_ADD;
            newnode->linepos = parser->current->linepos;
            newnode->binaryopleft = result;
            advance(parser);
            newnode->binaryopright = malloc(sizeof(struct Node));
            newnode->binaryopright->type = NODE_NEG; //x - y == x + (-y)
            newnode->binaryopright->linepos = parser->current->linepos;
            newnode->binaryopright->unaryopexpr = parsehigharithmeticop(parser);

            result = newnode;
        }
    }

    return result;
}

static struct Node* parsehighlogicalop(struct Parser* parser) {
    struct Node* result = parselowarithmeticop(parser);

    while (parser->current != NULL && (parser->current->type == TOK_EQ || parser->current->type == TOK_NEQ || parser->current->type == TOK_LESS || parser->current->type == TOK_GREATER || parser->current->type == TOK_LE || parser->current->type == TOK_GE)) {
        if (parser->current->type == TOK_EQ) {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_EQ;
            newnode->linepos = parser->current->linepos;
            newnode->binaryopleft = result;
            advance(parser);
            newnode->binaryopright = parselowarithmeticop(parser);

            result = newnode;
        }
        else if (parser->current->type == TOK_NEQ) {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_EQ;
            newnode->linepos = parser->current->linepos;
            newnode->binaryopleft = result;
            advance(parser);
            newnode->binaryopright = malloc(sizeof(struct Node));
            newnode->binaryopright->type = NODE_NOT; //x != y == x == !(y)
            newnode->binaryopright->linepos = parser->current->linepos;
            newnode->binaryopright->unaryopexpr = parselowarithmeticop(parser);

            result = newnode;
        }
        else if (parser->current->type == TOK_LESS) {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_LESS;
            newnode->linepos = parser->current->linepos;
            newnode->binaryopleft = result;
            advance(parser);
            newnode->binaryopright = parselowarithmeticop(parser);

            result = newnode;
        }
        else if (parser->current->type == TOK_GREATER) {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_NOT; //x > y == !(x < y)
            newnode->linepos = parser->current->linepos;
            newnode->unaryopexpr = malloc(sizeof(struct Node));
            newnode->unaryopexpr->type = NODE_LESS;
            newnode->unaryopexpr->linepos = parser->current->linepos;
            newnode->unaryopexpr->binaryopleft = result;
            advance(parser);
            newnode->unaryopexpr->binaryopright = parselowarithmeticop(parser);

            result = newnode;
        }
        else if (parser->current->type == TOK_LE) {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_NOT; //x <= y == (x < y) || (x == y) == !(!(x < y) && !(x == y))
            newnode->linepos = parser->current->linepos;
            newnode->unaryopexpr = malloc(sizeof(struct Node));
            newnode->unaryopexpr->type = NODE_AND; 
            newnode->unaryopexpr->linepos = parser->current->linepos;
            newnode->unaryopexpr->binaryopleft = malloc(sizeof(struct Node));
            newnode->unaryopexpr->binaryopleft->type = NODE_NOT;
            newnode->unaryopexpr->binaryopleft->linepos = parser->current->linepos;
            newnode->unaryopexpr->binaryopleft->unaryopexpr = malloc(sizeof(struct Node));
            newnode->unaryopexpr->binaryopleft->unaryopexpr->type = NODE_LESS;
            newnode->unaryopexpr->binaryopleft->unaryopexpr->linepos = parser->current->linepos;
            newnode->unaryopexpr->binaryopleft->unaryopexpr->binaryopleft = result;
            advance(parser);
            struct Node* right = parselowarithmeticop(parser);
            newnode->unaryopexpr->binaryopleft->unaryopexpr->binaryopright = right; 
            newnode->unaryopexpr->binaryopright = malloc(sizeof(struct Node));
            newnode->unaryopexpr->binaryopright->type = NODE_NOT;
            newnode->unaryopexpr->binaryopright->linepos = parser->current->linepos;
            newnode->unaryopexpr->binaryopright->unaryopexpr = malloc(sizeof(struct Node));
            newnode->unaryopexpr->binaryopright->unaryopexpr->type = NODE_EQ;
            newnode->unaryopexpr->binaryopright->unaryopexpr->linepos = parser->current->linepos;
            newnode->unaryopexpr->binaryopright->unaryopexpr->binaryopleft = result;
            newnode->unaryopexpr->binaryopright->unaryopexpr->binaryopright = right;

            result = newnode;
        }
        else {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_NOT; //x >= y == (!(x > y)) || (x == y) == !((x < y) && !(x == y))
            newnode->linepos = parser->current->linepos;
            newnode->unaryopexpr = malloc(sizeof(struct Node));
            newnode->unaryopexpr->type = NODE_AND;
            newnode->unaryopexpr->linepos = parser->current->linepos;
            newnode->unaryopexpr->binaryopleft = malloc(sizeof(struct Node));
            newnode->unaryopexpr->binaryopleft->type = NODE_LESS;
            newnode->unaryopexpr->binaryopleft->binaryopleft = result;
            advance(parser);
            struct Node* right = parselowarithmeticop(parser);
            newnode->unaryopexpr->binaryopleft->binaryopright = right;
            newnode->unaryopexpr->binaryopright = malloc(sizeof(struct Node));
            newnode->unaryopexpr->binaryopright->type = NODE_NOT;
            newnode->unaryopexpr->binaryopright->linepos = parser->current->linepos;
            newnode->unaryopexpr->binaryopright->unaryopexpr = malloc(sizeof(struct Node));
            newnode->unaryopexpr->binaryopright->unaryopexpr->type = NODE_EQ;
            newnode->unaryopexpr->binaryopright->unaryopexpr->linepos = parser->current->linepos;
            newnode->unaryopexpr->binaryopright->unaryopexpr->binaryopleft = result;
            newnode->unaryopexpr->binaryopright->unaryopexpr->binaryopright = right;

            result = newnode;
        }
    }

    return result;
}

static struct Node* parselowlogicalop(struct Parser* parser) {
    struct Node* result = parsehighlogicalop(parser);

    while (parser->current != NULL && (parser->current->type == TOK_AND || parser->current->type == TOK_OR)) {
        if (parser->current->type == TOK_AND) {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_AND;
            newnode->linepos = parser->current->linepos;
            newnode->binaryopleft = result;
            advance(parser);
            newnode->binaryopright = parsehighlogicalop(parser);

            result = newnode;
        }
        else {
            struct Node* newnode = malloc(sizeof(struct Node));
            newnode->type = NODE_NOT; //x || y == !(!x && !y)
            newnode->linepos = parser->current->linepos;
            newnode->unaryopexpr = malloc(sizeof(struct Node));
            newnode->unaryopexpr->type = NODE_AND;
            newnode->unaryopexpr->linepos = parser->current->linepos;
            newnode->unaryopexpr->binaryopleft = malloc(sizeof(struct Node));
            newnode->unaryopexpr->binaryopleft->type = NODE_NOT;
            newnode->unaryopexpr->binaryopleft->linepos = parser->current->linepos;
            newnode->unaryopexpr->binaryopleft->unaryopexpr = result;
            newnode->unaryopexpr->binaryopright = malloc(sizeof(struct Node));
            newnode->unaryopexpr->binaryopright->type = NODE_NOT;
            newnode->unaryopexpr->binaryopright->linepos = parser->current->linepos;
            advance(parser);
            newnode->unaryopexpr->binaryopright->unaryopexpr = parsehighlogicalop(parser);

            result = newnode;
        }
    }
    
    return result;
}

static struct Node* parseexpr(struct Parser* parser) {
    struct Node* result = parselowlogicalop(parser);

    if (parser->current == NULL) {
        printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos], parser->previous->value);
        exit(1);
    }
    else {
        if (parser->current->type == TOK_NUMBER || parser->current->type == TOK_STRING || parser->current->type == TOK_NAME) {
            printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos], parser->current->value);
            exit(1);
        }
        else {
            return result;
        }
    }
}   

static struct Vector* parsebody(struct Parser* parser, int istop, int isfunction, int isloop) {
    //init result vector
    struct Vector* result = vector(); 

    //parse body
    for (;;) {
        int isadvance = 1; //to handle with if statement without else

        if (parser->current == NULL) {
            if (istop) {
                break;
            }
            else {
                printf("At line %d:\n\t%s\nUnclosed \'{\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                exit(1);
            }
        }

        if (parser->current->type == TOK_END) {
            if (istop) {
                printf("At line %d:\n\t%s\nInvalid syntax: \'}\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                exit(1);
            }
            else {
                break;
            }
        }

        if (parser->current->type == TOK_VAR) { 
            int varlinepos = parser->current->linepos;

            advance(parser);

            if (parser->current == NULL) {
                printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos], parser->previous->value);
                exit(1);
            }
            else {
                if (parser->current->type != TOK_NAME) {
                    printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos], parser->current->value);
                    exit(1);
                }
                else {
                    char* name = malloc((strlen(parser->current->value) + 1) * sizeof(char)); //var name
                    strcpy(name, parser->current->value);
                    
                    advance(parser);
                    
                    if (parser->current == NULL) {
                        printf("At line %d:\n\t%s\nMissing \';\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                        exit(1);
                    }
                    else {
                        if (parser->current->type == TOK_ENDL) { //when no expr
                            struct Node* newnode = malloc(sizeof(struct Node));
                            newnode->type = NODE_VAR;
                            newnode->linepos = varlinepos;
                            newnode->varname = name;
                            newnode->varexpr = NULL;

                            push(result, newnode);
                        }
                        else if (parser->current->type == TOK_ASSIGN) {
                            advance(parser);
                            struct Node* expr = parseexpr(parser);

                            if (parser->current == NULL) {
                                printf("At line %d:\n\t%s\nMissing \';\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                exit(1);
                            }
                            else {
                                if (parser->current->type != TOK_ENDL) {
                                    printf("At line %d:\n\t%s\nMissing \';\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                    exit(1);
                                }
                                else {
                                    struct Node* newnode = malloc(sizeof(struct Node));
                                    newnode->type = NODE_VAR;
                                    newnode->linepos = varlinepos;
                                    newnode->varname = name;
                                    newnode->varexpr = expr;

                                    push(result, newnode);
                                }
                            }
                        }
                        else {
                            printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos], parser->current->value);
                            exit(1);
                        }
                    }
                }
            }
        }
        else if (parser->current->type == TOK_ARRAY) {
            int arraylinepos = parser->current->linepos;

            advance(parser);

            if (parser->current == NULL) {
                printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos], parser->previous->value);
                exit(1);
            }
            else {
                if (parser->current->type != TOK_NAME) {
                    printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos], parser->current->value);
                    exit(1);
                }
                else {
                    char* name = malloc((strlen(parser->current->value) + 1) * sizeof(char)); //array name
                    strcpy(name, parser->current->value);
                    
                    advance(parser);
                    
                    if (parser->current == NULL) {
                        printf("At line %d:\n\t%s\nMissing \'[\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                        exit(1);
                    }
                    else {
                        if (parser->current->type != TOK_LINDEX) {
                            printf("At line %d:\n\t%s\nMissing \'[\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                            exit(1);
                        }
                        else {
                            advance(parser);

                            if (parser->current == NULL) {
                                printf("At line %d:\n\t%s\nUnclosed \'[\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                exit(1);
                            }
                            else {
                                if (parser->current->type == TOK_RINDEX) {
                                    printf("At line %d:\n\t%s\nInvalid syntax: \']\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                                    exit(1);
                                }
                                else {
                                    struct Node* sizeexpr = parseexpr(parser);

                                    if (parser->current == NULL) {
                                        printf("At line %d:\n\t%s\nUnclosed \'[\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                        exit(1);
                                    }
                                    else {
                                        if (parser->current->type != TOK_RINDEX) {
                                            printf("At line %d:\n\t%s\nUnclosed \'[\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                                            exit(1);
                                        }
                                        else {
                                            advance(parser);

                                            if (parser->current == NULL) {
                                                printf("At line %d:\n\t%s\nMissing \';\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                                exit(1);
                                            }
                                            else {
                                                if (parser->current->type != TOK_ENDL) {
                                                    printf("At line %d:\n\t%s\nMissing \';\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                                                    exit(1);
                                                }
                                                else {
                                                    struct Node* newnode = malloc(sizeof(struct Node));
                                                    newnode->type = NODE_ARRAY;
                                                    newnode->linepos = arraylinepos;
                                                    newnode->arrayname= name;
                                                    newnode->arraysize = sizeexpr;

                                                    push(result, newnode);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (parser->current->type == TOK_IF) {
            int iflinepos = parser->current->linepos;

            advance(parser);

            if (parser->current == NULL) {
                printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos], parser->previous->value);
                exit(1);
            }
            else {
                struct Node* condition = parseexpr(parser);

                if (parser->current == NULL) {
                    printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                    exit(1);
                }
                else {
                    if (parser->current->type != TOK_BEGIN) {
                        printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                        exit(1);
                    }
                    else {
                        advance(parser);

                        struct Vector* body = parsebody(parser, 0, isfunction, 1);
                        struct Node* newnode = malloc(sizeof(struct Node));
                        newnode->type = NODE_IF;
                        newnode->linepos = iflinepos;
                        newnode->condition = condition;
                        newnode->condbody = body;
                        newnode->elsebody = vector();

                        struct Node* topnode = newnode;
                        struct Node* prevnode = newnode;

                        isadvance = 0;

                        for (;;) {
                            advance(parser);

                            if (parser->current == NULL) {
                                break;
                            }
                            else {
                                if (parser->current->type == TOK_ELIF) {
                                    int eliflinepos = parser->current->linepos;

                                    advance(parser);

                                    if (parser->current == NULL) {
                                        printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos], parser->previous->value);
                                        exit(1);
                                    }
                                    else {
                                        struct Node* condition = parseexpr(parser);

                                        if (parser->current == NULL) {
                                            printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                            exit(1);
                                        }
                                        else {
                                            if (parser->current->type != TOK_BEGIN) {
                                                printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                                                exit(1);
                                            }
                                            else {
                                                advance(parser);

                                                struct Vector* body = parsebody(parser, 0, isfunction, 1);
                                                struct Node* newnode = malloc(sizeof(struct Node));
                                                newnode->type = NODE_IF;
                                                newnode->linepos = eliflinepos;
                                                newnode->condition = condition;
                                                newnode->condbody = body;
                                                newnode->elsebody = vector();

                                                push(prevnode->elsebody, newnode);

                                                prevnode = newnode;
                                            }
                                        }
                                    }
                                }
                                else if (parser->current->type == TOK_ELSE) {
                                    advance(parser);

                                    if (parser->current == NULL) {
                                            printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                            exit(1);
                                    }
                                    else {
                                        if (parser->current->type != TOK_BEGIN) {
                                            printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                                            exit(1);
                                        }
                                        else {
                                            advance(parser);

                                            prevnode->elsebody = parsebody(parser, 0, isfunction, 1);
                                            advance(parser);
                                            break;
                                        }
                                    }
                                }
                                else {
                                    break;
                                }
                            }
                        }
                        push(result, topnode);
                    }
                }
            }
        }
        else if (parser->current->type == TOK_WHILE) {
            int whilelinepos = parser->current->linepos;

            advance(parser);

            if (parser->current == NULL) {
                printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos], parser->previous->value);
                exit(1);
            }
            else {
                struct Node* condition = parseexpr(parser);

                if (parser->current == NULL) {
                    printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                    exit(1);
                }
                else {
                    if (parser->current->type != TOK_BEGIN) {
                        printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                        exit(1);
                    }
                    else {
                        advance(parser);

                        struct Vector* body = parsebody(parser, 0, isfunction, 1);
                        struct Node* newnode = malloc(sizeof(struct Node));
                        newnode->type = NODE_WHILE;
                        newnode->linepos = whilelinepos;
                        newnode->condition = condition;
                        newnode->condbody = body;
                        newnode->elsebody = NULL;

                        push(result, newnode);
                    }
                }
            }
        }
        else if (parser->current->type == TOK_BREAK) {
            if (isloop) {
                advance(parser);

                if (parser->current == NULL) {
                    printf("At line %d:\n\t%s\nMissing \';\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                    exit(1);
                }
                else {
                    if (parser->current->type != TOK_ENDL) {
                        printf("At line %d:\n\t%s\nMissing \';\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                        exit(1);
                    }
                    else {
                        struct Node* newnode = malloc(sizeof(struct Node));
                        newnode->type = NODE_BREAK;

                        push(result, newnode);
                    }
                }
            }
            else {
                printf("At line %d:\n\t%s\n'break' outside a loop\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                exit(1);
            }
        }
        else if (parser->current->type == TOK_FUNCTION) {
            int functionlinepos = parser->current->linepos;

            advance(parser);

            if (parser->current == NULL) {
                printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos], parser->previous->value);
                exit(1);
            }
            else {
                if (parser->current->type != TOK_NAME) {
                    printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos], parser->current->value);
                    exit(1);
                }
                else {
                    char* name = malloc((strlen(parser->current->value) + 1) * sizeof(char)); //function name
                    strcpy(name, parser->current->value);
                    
                    advance(parser);

                    if (parser->current == NULL) {
                        printf("At line %d:\n\t%s\nMissing \'(\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                        exit(1);
                    }
                    else {
                        if (parser->current->type != TOK_LPAREN) {
                            printf("At line %d:\n\t%s\nMissing \'(\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                            exit(1);
                        }
                        else {
                            struct Vector* param = vector();

                            for(;;) {
                                advance(parser);
                                if (parser->current == NULL) {
                                    printf("At line %d:\n\t%s\nUnclosed \'(\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                    exit(1);
                                }
                                else {
                                    if (parser->current->type == TOK_RPAREN) {
                                        break;
                                    }
                                    
                                    if (parser->current->type == TOK_NAME) {
                                        char* paramname = malloc((strlen(parser->current->value) + 1) * sizeof(char));
                                        strcpy(paramname, parser->current->value);
                                        push(param, paramname);
                                        advance(parser);

                                        if (parser->current == NULL) {
                                            printf("At line %d:\n\t%s\nUnclosed \'(\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                            exit(1);
                                        }
                                        else {
                                            if (parser->current->type == TOK_RPAREN) {
                                                break;
                                            }
                                            if (parser->current->type != TOK_COMMA) {
                                                printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos], parser->current->value);
                                                exit(1);
                                            }
                                        }
                                    }
                                }
                            }
                            advance(parser);

                            if (parser->current == NULL) {
                                printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                exit(1);
                            }
                            else {
                                if (parser->current->type != TOK_BEGIN) {
                                    printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                                    exit(1);
                                }
                                else {
                                    advance(parser);

                                    struct Vector* body = parsebody(parser, 0, 1, 0);
                                    struct Node* newnode = malloc(sizeof(struct Node));
                                    newnode->type = NODE_FUNCTION;
                                    newnode->linepos = functionlinepos;
                                    newnode->invokablename = name;
                                    newnode->invokableparam = param;
                                    newnode->invokablebody = body;

                                    push(result, newnode);
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (parser->current->type == TOK_RETURN) {
            if (isfunction) {
                advance(parser);
                struct Node* expr = parseexpr(parser);

                if (parser->current == NULL) {
                    printf("At line %d:\n\t%s\nMissing \';\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                    exit(1);
                }
                else {
                    if (parser->current->type != TOK_ENDL) {
                        printf("At line %d:\n\t%s\nMissing \';\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                        exit(1);
                    }
                    else {
                        struct Node* newnode = malloc(sizeof(struct Node));
                        newnode->type = NODE_RETURN;
                        newnode->returnexpr = expr;

                        push(result, newnode);
                    }
                }
            }
            else {
                printf("At line %d:\n\t%s\n'return' outside a function\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                exit(1);
            }
        }
        else if (parser->current->type == TOK_CLASS) {
            int classlinepos = parser->current->linepos;

            advance(parser);

            if (parser->current == NULL) {
                printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos], parser->previous->value);
                exit(1);
            }
            else {
                if (parser->current->type != TOK_NAME) {
                    printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos], parser->current->value);
                    exit(1);
                }
                else {
                    char* name = malloc((strlen(parser->current->value) + 1) * sizeof(char)); //class name
                    strcpy(name, parser->current->value);
                    
                    advance(parser);

                    if (parser->current == NULL) {
                        printf("At line %d:\n\t%s\nMissing \'(\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                        exit(1);
                    }
                    else {
                        if (parser->current->type != TOK_LPAREN) {
                            printf("At line %d:\n\t%s\nMissing \'(\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                            exit(1);
                        }
                        else {
                            struct Vector* param = vector();

                            for(;;) {
                                advance(parser);
                                if (parser->current == NULL) {
                                    printf("At line %d:\n\t%s\nUnclosed \'(\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                    exit(1);
                                }
                                else {
                                    if (parser->current->type == TOK_RPAREN) {
                                        break;
                                    }
                                    
                                    if (parser->current->type == TOK_NAME) {
                                        char* paramname = malloc((strlen(parser->current->value) + 1) * sizeof(char));
                                        strcpy(paramname, parser->current->value);
                                        push(param, paramname);
                                        advance(parser);

                                        if (parser->current == NULL) {
                                            printf("At line %d:\n\t%s\nUnclosed \'(\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                            exit(1);
                                        }
                                        else {
                                            if (parser->current->type == TOK_RPAREN) {
                                                break;
                                            }
                                            if (parser->current->type != TOK_COMMA) {
                                                printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos], parser->current->value);
                                                exit(1);
                                            }
                                        }
                                    }
                                }
                            }
                            advance(parser);

                            if (parser->current == NULL) {
                                printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                                exit(1);
                            }
                            else {
                                if (parser->current->type != TOK_BEGIN) {
                                    printf("At line %d:\n\t%s\nMissing \'{\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos]);
                                    exit(1);
                                }
                                else {
                                    advance(parser);

                                    struct Vector* body = parsebody(parser, 0, 0, 0);
                                    struct Node* newnode = malloc(sizeof(struct Node));
                                    newnode->type = NODE_CLASS;
                                    newnode->linepos = classlinepos;
                                    newnode->invokablename = name;
                                    newnode->invokableparam = param;
                                    newnode->invokablebody = body;

                                    push(result, newnode);
                                }
                            }
                        }
                    }
                }
            }
        }
        else {
            int exprlinepos = parser->current->linepos;

            struct Node* expr = parseexpr(parser);

            if (parser->current == NULL) {
                printf("At line %d:\n\t%s\nMissing \';\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                exit(1);
            }
            else {
                if (parser->current->type == TOK_ASSIGN) {
                    if (expr->type != NODE_NAME && expr->type != NODE_ATTR && expr->type != NODE_INDEX) {
                        printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos], parser->previous->value);
                        exit(1);
                    } 
                    else {
                        advance(parser);
                        
                        struct Node* assignexpr = parseexpr(parser);

                        if (parser->current == NULL) {
                            printf("At line %d:\n\t%s\nMissing \';\'\n", parser->previous->linepos + 1, parser->lines->body[parser->previous->linepos]);
                            exit(1);
                        }
                        else {
                            if (parser->current->type == TOK_ENDL) {
                                struct Node* newnode = malloc(sizeof(struct Node));
                                newnode->type = NODE_ASSIGN;
                                newnode->linepos = exprlinepos;
                                newnode->assignnameexpr = expr;
                                newnode->assignexpr = assignexpr;

                                push(result, newnode);
                            }
                        }
                    }
                }
                else if (parser->current->type == TOK_ENDL) {
                    struct Node* newnode = malloc(sizeof(struct Node));
                    newnode->type = NODE_EXPR;
                    newnode->linepos = exprlinepos;
                    newnode->expr = expr;

                    push(result, newnode);
                }
                else {
                    printf("At line %d:\n\t%s\nInvalid syntax: \'%s\'\n", parser->current->linepos + 1, parser->lines->body[parser->current->linepos], parser->current->value);
                    exit(1);
                }
            }
        }

        if (isadvance) advance(parser);
    }

    return result;
    
}

struct Parser* parse(struct Lexer* lexer) {
    //init parser
    struct Parser* parser = malloc(sizeof(struct Parser));
    parser->tokens = lexer->result;
    parser->lines = lexer->lines;
    parser->pos = 0;
    parser->previous = NULL;
    parser->current = (struct Token*)lexer->result->body[0];

    //init result vector
    parser->result = parsebody(parser, 1, 0, 0);

    //free tokens
    for (int i = 0; i <= lexer->result->last; i++) {
        struct Token* token = lexer->result->body[i];
        safefree(token->value);
        safefree(token);
    }
    
    //free lexer
    safefree(lexer->result);
    safefree(lexer);

    return parser;
}
