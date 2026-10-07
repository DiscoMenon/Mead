#include "parser.h"

#include <stdexcept>
#include <string>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {
}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::advance() {
    return tokens[current++];
}

bool Parser::check(TokenType type) const {
    return peek().type == type;
}

const Token& Parser::consume(
    TokenType type,
    const std::string& message
) {
    if (check(type)) {
        return advance();
    }

    throw std::runtime_error(message);
}

std::unique_ptr<Expr> Parser::parseComparison() {

    auto expression = parseAddition();

    while (
        check(TokenType::LESS) ||
        check(TokenType::LESS_EQUAL) ||
        check(TokenType::GREATER) ||
        check(TokenType::GREATER_EQUAL) ||
        check(TokenType::EQUAL_EQUAL) ||
        check(TokenType::NOT_EQUAL)
    ) {
        Token op = advance();

        auto right = parseAddition();

        expression = std::make_unique<BinaryExpr>(
            std::move(expression),
            op.value,
            std::move(right)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {

    auto expression = parseComparison();

    while (check(TokenType::AND)) {
        Token op = advance();

        auto right = parseComparison();

        expression = std::make_unique<BinaryExpr>(
            std::move(expression),
            op.value,
            std::move(right)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseLogicalOr() {

    auto expression = parseLogicalAnd();

    while (check(TokenType::OR)) {
        Token op = advance();

        auto right = parseLogicalAnd();

        expression = std::make_unique<BinaryExpr>(
            std::move(expression),
            op.value,
            std::move(right)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parsePrimary() {

    if (check(TokenType::INTEGER)) {
        int value = std::stoi(advance().value);
        return std::make_unique<IntegerExpr>(value);
    }

    if (check(TokenType::STRING)) {
        return std::make_unique<StringExpr>(advance().value);
    }

    if (check(TokenType::FLOAT)) {
        double value = std::stod(advance().value);
        return std::make_unique<FloatExpr>(value);
    }

    if (check(TokenType::BOOL)) {
        bool value = advance().value == "true";
        return std::make_unique<BoolExpr>(value);
    }

    if (check(TokenType::IDENTIFIER)) {
        return std::make_unique<VariableExpr>(advance().value);
    }

    if (check(TokenType::LPAREN)) {
        advance();

        auto expression = parseExpression();

        consume(
            TokenType::RPAREN,
            "Expected ')' after expression"
        );

        return expression;
    }

    throw std::runtime_error("Expected expression");
}

std::unique_ptr<Expr> Parser::parseMultiplication() {

    auto expression = parsePrimary();

    while (
        check(TokenType::STAR) ||
        check(TokenType::SLASH) ||
        check(TokenType::PERCENT)
    ) {
        Token op = advance();

        auto right = parsePrimary();

        expression = std::make_unique<BinaryExpr>(
            std::move(expression),
            op.value,
            std::move(right)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseAddition() {

    auto expression = parseMultiplication();

    while (
        check(TokenType::PLUS) ||
        check(TokenType::MINUS)
    ) {
        Token op = advance();

        auto right = parseMultiplication();

        expression = std::make_unique<BinaryExpr>(
            std::move(expression),
            op.value,
            std::move(right)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseLogicalOr();
}

std::unique_ptr<Stmt> Parser::parsePrint() {

    advance(); // print

    consume(
        TokenType::LPAREN,
        "Expected '(' after print"
    );

    auto expression = parseExpression();

    consume(
        TokenType::RPAREN,
        "Expected ')' after expression"
    );

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after print statement"
    );

    return std::make_unique<PrintStmt>(
        std::move(expression)
    );
}

std::unique_ptr<Stmt> Parser::parseVarDeclaration() {

    advance(); // var

    const Token& name = consume(
        TokenType::IDENTIFIER,
        "Expected variable name after 'var'"
    );

    consume(
        TokenType::EQUAL,
        "Expected '=' after variable name"
    );

    auto initializer = parseExpression();

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after variable declaration"
    );

    return std::make_unique<VarDeclStmt>(
        name.value,
        std::move(initializer)
    );
}

std::unique_ptr<Stmt> Parser::parseStatement() {

    if (check(TokenType::PRINT)) {
        return parsePrint();
    }

    if (check(TokenType::IF)) {
        return parseIf();
    }

    if (check(TokenType::VAR)) {
        return parseVarDeclaration();
    }

    throw std::runtime_error("Expected statement");
}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {

    std::vector<std::unique_ptr<Stmt>> statements;

    while (!check(TokenType::END_OF_FILE)) {
        statements.push_back(parseStatement());
    }

    return statements;
}

std::vector<std::unique_ptr<Stmt>> Parser::parseBlock() {

    consume(
        TokenType::LBRACE,
        "Expected '{'"
    );

    std::vector<std::unique_ptr<Stmt>> statements;

    while (!check(TokenType::RBRACE) &&
           !check(TokenType::END_OF_FILE)) {

        statements.push_back(parseStatement());
    }

    consume(
        TokenType::RBRACE,
        "Expected '}' after block"
    );

    return statements;
}

std::unique_ptr<Stmt> Parser::parseIf() {

    advance(); // if

    consume(
        TokenType::LPAREN,
        "Expected '(' after 'if'"
    );

    auto condition = parseExpression();

    consume(
        TokenType::RPAREN,
        "Expected ')' after condition"
    );

    auto thenBranch = parseBlock();

    std::vector<std::unique_ptr<Stmt>> elseBranch;

    if (check(TokenType::ELSE)) {
        advance();

        elseBranch = parseBlock();
    }

    return std::make_unique<IfStmt>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch)
    );
}