#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

#include "lex.h"

#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define isalpha(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || ((c) == '_')) 

#define char2str(c) ((char[]){(c), (0)})
#define addchar2str(s, c)\
    s = realloc((s), (strlen((s)) + 2) * sizeof(char));\
    strcat((s), char2str((c)));\

#define safefree(p)\
    free(p);\
    p = NULL

static void advance(struct Lexer* lexer) {
    ++lexer->pos;
    lexer->current = lexer->code[lexer->pos];
}

static void createtoken(struct Vector* vector, enum TokenType type, char* value, int line) {
    struct Token* token = malloc(sizeof(struct Token));
    token->type = type;
    
    char* tokenvalue = malloc((strlen(value) + 1) * sizeof(char));
    strcpy(tokenvalue, value);
    token->value = tokenvalue;
    
    token->linepos = line;
    push(vector, token);
}

struct Lexer* lex(char* code) {
    //init lexer
    struct Lexer* lexer = malloc(sizeof(struct Lexer));
    lexer->code = code;
    lexer->pos = 0;
    lexer->linepos = 0;
    lexer->current = code[0];

    //init result vector
    lexer->result = vector();

    //init lines vector
    lexer->lines = vector();

    //split code to lines vector
    char* line = calloc(1, sizeof(char)); //init line
    int pos = 0;

    while (pos <= strlen(code)) {
        if (code[pos] == '\n' || code[pos] == '\0') {
            char* linevalue = malloc((strlen(line) + 1) * sizeof(char));
            strcpy(linevalue, line);
            push(lexer->lines, linevalue);
            line = calloc(1, sizeof(char)); //init line
            ++pos;

            while (code[pos] == '\t') ++pos; //remove '\t' in the front of line
        }
        else {
            addchar2str(line, code[pos]);
            ++pos;
        }
    }

    safefree(line);

    char* value = calloc(1, sizeof(char)); //init value

    for (;;) {
        if (lexer->pos > strlen(code)) break;
        if (lexer->current == ' ' || lexer->current == '\t') {
            advance(lexer);
        }
        else if (lexer->current == '\n' || lexer->current == '\0') {
            ++lexer->linepos;
            advance(lexer);
        }
        //comment
        else if (lexer->current == '/' && lexer->code[lexer->pos + 1] == '/') {
            advance(lexer);
            while (lexer->current != '\n' && lexer->current != '\0') {
                advance(lexer);
            }
        }
        
        else if (lexer->current == '"') {
            advance(lexer);
            while (lexer->current != '"') {
                //todo: escape character
                if (lexer->pos > strlen(code)) {
                    printf("At line %d:\n\t%s\nInvalid syntax: Unclosed \'\"\'\n", lexer->linepos + 1, lexer->lines->body[lexer->linepos]);
                    exit(1);
                }
                if (lexer->current == '\n') ++lexer->linepos;
                addchar2str(value, lexer->current);
                advance(lexer);
            }
            advance(lexer);

            createtoken(lexer->result, TOK_STRING, value, lexer->linepos);
            value = calloc(1, sizeof(char)); //init value
        }

        else if (lexer->current == '=') {
            if (lexer->code[lexer->pos + 1] == '=') { //eq
                advance(lexer);
                createtoken(lexer->result, TOK_EQ, "==", lexer->linepos);
                advance(lexer);
            }
            else { //assign
                createtoken(lexer->result, TOK_ASSIGN, "=", lexer->linepos);
                advance(lexer);
            }
        }
        //end line
        else if (lexer->current == ';') {
            createtoken(lexer->result, TOK_ENDL, ";", lexer->linepos);
            advance(lexer);
        }
        //begin
        else if (lexer->current == '{') {
            createtoken(lexer->result, TOK_BEGIN, "{", lexer->linepos);
            advance(lexer);
        }
        //endl
        else if (lexer->current == ';') {
            createtoken(lexer->result, TOK_ENDL, ";", lexer->linepos);
            advance(lexer);
        }
        //lparen
        else if (lexer->current == '(') {
            createtoken(lexer->result, TOK_LPAREN, "(", lexer->linepos);
            advance(lexer);
        }
        //rparen
        else if (lexer->current == ')') {
            createtoken(lexer->result, TOK_RPAREN, ")", lexer->linepos);
            advance(lexer);
        }
        //lindex
        else if (lexer->current == '[') {
            createtoken(lexer->result, TOK_LINDEX, "[", lexer->linepos);
            advance(lexer);
        }
        //rindex
        else if (lexer->current == ']') {
            createtoken(lexer->result, TOK_RINDEX, "]", lexer->linepos);
            advance(lexer);
        }
        //begin
        else if (lexer->current == '{') {
            createtoken(lexer->result, TOK_BEGIN, "{", lexer->linepos);
            advance(lexer);
        }
        //end
        else if (lexer->current == '}') {
            createtoken(lexer->result, TOK_END, "}", lexer->linepos);
            advance(lexer);
        }
        //attr
        else if (lexer->current == '.') {
            createtoken(lexer->result, TOK_ATTR, ".", lexer->linepos);
            advance(lexer);
        }
        //comma
        else if (lexer->current == ',') {
            createtoken(lexer->result, TOK_COMMA, ",", lexer->linepos);
            advance(lexer);
        }

        //plus
        else if (lexer->current == '+') {
            createtoken(lexer->result, TOK_PLUS, "+", lexer->linepos);
            advance(lexer);
        }
        //minus
        else if (lexer->current == '-') {
            createtoken(lexer->result, TOK_MINUS, "-", lexer->linepos);
            advance(lexer);
        }
        //mul
        else if (lexer->current == '*') {
            createtoken(lexer->result, TOK_MUL, "*", lexer->linepos);
            advance(lexer);
        }
        //div
        else if (lexer->current == '/') {
            createtoken(lexer->result, TOK_DIV, "/", lexer->linepos);
            advance(lexer);
        }
        else if (lexer->current == '!') {
            if (lexer->code[lexer->pos + 1] == '=') { //neq
                advance(lexer);
                createtoken(lexer->result, TOK_NEQ, "!=", lexer->linepos);
                advance(lexer);
            }
            else { //not
                createtoken(lexer->result, TOK_NOT, "!", lexer->linepos);
                advance(lexer);
            }
        }
        //and
        else if (lexer->current == '&' && lexer->code[lexer->pos + 1] == '&') {
            advance(lexer);
            createtoken(lexer->result, TOK_AND, "&&", lexer->linepos);
            advance(lexer);
        }
        //or
        else if (lexer->current == '|' && lexer->code[lexer->pos + 1] == '|') {
            advance(lexer);
            createtoken(lexer->result, TOK_OR, "||", lexer->linepos);
            advance(lexer);
        }
        else if (lexer->current == '<') {
            if (lexer->code[lexer->pos + 1] == '=') { //le
                advance(lexer);
                createtoken(lexer->result, TOK_LE, "<=", lexer->linepos);
                advance(lexer);
            }
            else { //less
                createtoken(lexer->result, TOK_LESS, "<", lexer->linepos);
                advance(lexer);
            }
        }
        else if (lexer->current == '>') {
            if (lexer->code[lexer->pos + 1] == '=') { //ge
                advance(lexer);
                createtoken(lexer->result, TOK_GE, ">=", lexer->linepos);
                advance(lexer);
            }
            else { //greater
                createtoken(lexer->result, TOK_GREATER, ">", lexer->linepos);
                advance(lexer);
            }
        }
        //length
        else if (lexer->current == '$') {
            createtoken(lexer->result, TOK_LENGTH, "$", lexer->linepos);
            advance(lexer);
        }

        //numbers
        else if (isdigit(lexer->current)) {
            while (isdigit(lexer->current)) {
                addchar2str(value, lexer->current);
                advance(lexer);
            }

            if (lexer->current == '.') {
                if (isdigit(lexer->code[lexer->pos + 1])) { 
                    advance(lexer);
                    addchar2str(value, '.');
                    while (isdigit(lexer->current)) {
                        addchar2str(value, lexer->current);
                        advance(lexer);
                    }
                    createtoken(lexer->result, TOK_NUMBER, value, lexer->linepos);
                    value = calloc(1, sizeof(char)); //init value
                }
                else { 
                    createtoken(lexer->result, TOK_NUMBER, value, lexer->linepos);
                    value = calloc(1, sizeof(char)); //init value
                }
            }
            else { 
                createtoken(lexer->result, TOK_NUMBER, value, lexer->linepos);
                value = calloc(1, sizeof(char)); //init value
            }
        }
        //keyword & name
        else if (isalpha(lexer->current)) {
            while (isalpha(lexer->current) || isdigit(lexer->current)) {
                addchar2str(value, lexer->current);
                advance(lexer);
            }

            //check keyword
            enum TokenType tokentype;

            if (!strcmp(value, "var")) tokentype = TOK_VAR;
            else if (!strcmp(value, "array")) tokentype = TOK_ARRAY;
            else if (!strcmp(value, "function")) tokentype = TOK_FUNCTION;
            else if (!strcmp(value, "return")) tokentype = TOK_RETURN;
            else if (!strcmp(value, "class")) tokentype = TOK_CLASS;
            else if (!strcmp(value, "if")) tokentype = TOK_IF;
            else if (!strcmp(value, "elif")) tokentype = TOK_ELIF;
            else if (!strcmp(value, "else")) tokentype = TOK_ELSE;
            else if (!strcmp(value, "while")) tokentype = TOK_WHILE;
            else if (!strcmp(value, "break")) tokentype = TOK_BREAK;
            else tokentype = TOK_NAME;

            createtoken(lexer->result, tokentype, value, lexer->linepos);
            value = calloc(1, sizeof(char)); //init value
        }
        else {
            printf("At line %d:\n\t%s\nInvalid syntax: \'%c\'\n", lexer->linepos + 1, lexer->lines->body[lexer->linepos], lexer->current);
            exit(1);
        }
    }

    safefree(value);
    
    return lexer;
}