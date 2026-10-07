#pragma once

#include <string>

enum class TokenType {
    // Keywords
    PRINT,
    VAR,

    // Literals
    INTEGER,
    FLOAT,
    BOOL,
    STRING,
    IDENTIFIER,

    // Symbols
    LPAREN,       // (
    RPAREN,       // )
    LBRACE,       // {
    RBRACE,       // }
    LBRACKET,     // [
    RBRACKET,     // ]

    PLUS,         // +
    MINUS,        // -
    STAR,         // *
    SLASH,        // /
    PERCENT,      // %

    EQUAL,        // =
    EQUAL_EQUAL,  // ==
    NOT_EQUAL,    // !=

    LESS,         // <
    LESS_EQUAL,   // <=
    GREATER,      // >
    GREATER_EQUAL,// >=

    AND,          // &&
    OR,           // ||
    NOT,          // !

    COLON,        // :
    COMMA,        // ,
    DOT,          // .
    SEMICOLON,    // ;

    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
};