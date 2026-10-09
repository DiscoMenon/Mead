#pragma once

#include "ast.h"
#include "token.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class Parser {
private:
    const std::vector<Token>& tokens;
    size_t current;

    const Token& peek() const;
    const Token& advance();
    bool check(TokenType type) const;

    const Token& consume(
        TokenType type,
        const std::string& message
    );

    std::unique_ptr<Stmt> parseStatement();

    std::unique_ptr<Stmt> parsePrint();
    std::unique_ptr<Stmt> parseVarDeclaration();
    std::unique_ptr<Stmt> parseAssignment();

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseAddition();
    std::unique_ptr<Expr> parseMultiplication();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseLogicalAnd();
    std::unique_ptr<Expr> parseLogicalOr();
    std::unique_ptr<Stmt> parseIf();
    std::vector<std::unique_ptr<Stmt>> parseBlock();
    std::unique_ptr<Stmt> parseWhile();
    

public:
    explicit Parser(const std::vector<Token>& tokens);

    std::vector<std::unique_ptr<Stmt>> parse();
};