#ifndef __LEX_H__
#define __LEX_H__

enum TokenType {
    TOK_NUMBER = 0,
    TOK_STRING,
    TOK_NAME,
    TOK_VAR,
    TOK_ASSIGN,
    TOK_ARRAY,
    TOK_FUNCTION,
    TOK_RETURN,
    TOK_CLASS,
    TOK_IF,
    TOK_ELIF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_BREAK,
    TOK_ENDL,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LINDEX,
    TOK_RINDEX,
    TOK_BEGIN,
    TOK_END,
    TOK_ATTR,
    TOK_COMMA,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_NOT,
    TOK_AND,
    TOK_OR,
    TOK_EQ,
    TOK_NEQ,
    TOK_LESS,
    TOK_GREATER,
    TOK_LE,
    TOK_GE,
    TOK_LENGTH
};

struct Token {
    enum TokenType type;
    char* value;
    int linepos;
};

struct Lexer {
    char* code;
    struct Vector* lines;
    struct Vector* result;

    int pos;
    int linepos;
    char current;
};

struct Lexer* lex(char* code);

#endif