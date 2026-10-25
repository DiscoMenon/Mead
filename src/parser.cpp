#include "parser.h"
#include "lexer.h"
#include <stdexcept>
#include <string>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}
const Token& Parser::peek() const { return tokens[current]; }
const Token& Parser::advance() { return tokens[current++]; }
bool Parser::check(TokenType type) const { return peek().type == type; }
const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw std::runtime_error(message);
}

std::unique_ptr<Expr> Parser::parseComparison() {
    auto expression = parseAddition();
    while (check(TokenType::LESS) || check(TokenType::LESS_EQUAL) ||
           check(TokenType::GREATER) || check(TokenType::GREATER_EQUAL) ||
           check(TokenType::EQUAL_EQUAL) || check(TokenType::NOT_EQUAL)) {
        Token op = advance(); auto right = parseAddition();
        expression = std::make_unique<BinaryExpr>(std::move(expression), op.value, std::move(right));
    }
    return expression;
}
std::unique_ptr<Expr> Parser::parseLogicalAnd() {
    auto expression = parseComparison();
    while (check(TokenType::AND)) { Token op = advance(); auto right = parseComparison(); expression = std::make_unique<BinaryExpr>(std::move(expression), op.value, std::move(right)); }
    return expression;
}
std::unique_ptr<Expr> Parser::parseLogicalOr() {
    auto expression = parseLogicalAnd();
    while (check(TokenType::OR)) { Token op = advance(); auto right = parseLogicalAnd(); expression = std::make_unique<BinaryExpr>(std::move(expression), op.value, std::move(right)); }
    return expression;
}
std::unique_ptr<Expr> Parser::parsePrimary() {
    if (check(TokenType::INTEGER) || check(TokenType::FLOAT)) {
        const bool isFloat = check(TokenType::FLOAT);
        const std::string number = advance().value;
        std::unique_ptr<Expr> value = isFloat
            ? std::unique_ptr<Expr>(std::make_unique<FloatExpr>(std::stod(number)))
            : std::unique_ptr<Expr>(std::make_unique<IntegerExpr>(std::stoi(number)));
        if (check(TokenType::DOT) && current + 1 < tokens.size() && tokens[current + 1].type == TokenType::IDENTIFIER) {
            advance(); const std::string unit = advance().value;
            long long scale = 1; std::string dimension;
            if (unit == "m") dimension = "L";
            else if (unit == "km") { dimension = "L"; scale = 1000; }
            else if (unit == "s") dimension = "T";
            else if (unit == "min") { dimension = "T"; scale = 60; }
            else if (unit == "h") { dimension = "T"; scale = 3600; }
            else throw std::runtime_error("Unknown unit: " + unit + " (supported: m, km, s, min, h)");
            return std::make_unique<UnitExpr>(std::move(value), unit, scale, dimension);
        }
        return value;
    }
    if (check(TokenType::STRING)) {
        const std::string value = advance().value;
        std::vector<InterpolatedStringPart> parts;
        size_t cursor = 0;
        bool hasInterpolation = false;

        while (cursor < value.size()) {
            const size_t start = value.find("${", cursor);
            if (start == std::string::npos) {
                if (cursor < value.size())
                    parts.push_back({value.substr(cursor), nullptr});
                break;
            }

            hasInterpolation = true;
            if (start > cursor)
                parts.push_back({value.substr(cursor, start - cursor), nullptr});

            const size_t end = value.find('}', start + 2);
            if (end == std::string::npos)
                throw std::runtime_error("Unterminated string interpolation; expected '}'");

            const std::string embedded = value.substr(start + 2, end - (start + 2));
            if (embedded.find_first_not_of(" \t\r\n") == std::string::npos)
                throw std::runtime_error("String interpolation cannot be empty");

            Lexer embeddedLexer(embedded);
            auto embeddedTokens = embeddedLexer.tokenize();
            Parser embeddedParser(embeddedTokens);
            auto expression = embeddedParser.parseExpressionOnly();
            parts.push_back({"", std::move(expression)});
            cursor = end + 1;
        }

        if (!hasInterpolation)
            return std::make_unique<StringExpr>(value);
        if (value.empty())
            parts.push_back({"", nullptr});
        return std::make_unique<InterpolatedStringExpr>(std::move(parts));
    }
    if (check(TokenType::FLOAT)) return std::make_unique<FloatExpr>(std::stod(advance().value));
    if (check(TokenType::BOOL)) return std::make_unique<BoolExpr>(advance().value == "true");
    if (check(TokenType::IDENTIFIER)) {
        std::string name = advance().value;
        if (!check(TokenType::LPAREN)) return std::make_unique<VariableExpr>(name);
        advance();
        std::vector<std::unique_ptr<Expr>> args;
        if (!check(TokenType::RPAREN)) {
            do {
                args.push_back(parseExpression());
                if (!check(TokenType::COMMA)) break;
                advance();
            } while (true);
        }
        consume(TokenType::RPAREN, "Expected ')' after function arguments");
        return std::make_unique<CallExpr>(name, std::move(args));
    }
    if (check(TokenType::LPAREN)) { advance(); auto e = parseExpression(); consume(TokenType::RPAREN, "Expected ')' after expression"); return e; }
    throw std::runtime_error("Expected expression");
}
std::unique_ptr<Expr> Parser::parseMultiplication() {
    auto expression = parsePrimary();
    while (check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::PERCENT)) { Token op = advance(); auto right = parsePrimary(); expression = std::make_unique<BinaryExpr>(std::move(expression), op.value, std::move(right)); }
    return expression;
}
std::unique_ptr<Expr> Parser::parseAddition() {
    auto expression = parseMultiplication();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) { Token op = advance(); auto right = parseMultiplication(); expression = std::make_unique<BinaryExpr>(std::move(expression), op.value, std::move(right)); }
    return expression;
}
std::unique_ptr<Expr> Parser::parseExpression() { return parseLogicalOr(); }
std::unique_ptr<Stmt> Parser::parsePrint() {
    advance(); consume(TokenType::LPAREN, "Expected '(' after print"); auto e = parseExpression(); consume(TokenType::RPAREN, "Expected ')' after expression"); consume(TokenType::SEMICOLON, "Expected ';' after print statement"); return std::make_unique<PrintStmt>(std::move(e));
}
std::unique_ptr<Stmt> Parser::parseVarDeclaration() {
    advance(); auto name = consume(TokenType::IDENTIFIER, "Expected variable name after 'var'"); consume(TokenType::EQUAL, "Expected '=' after variable name"); auto value = parseExpression(); consume(TokenType::SEMICOLON, "Expected ';' after variable declaration"); return std::make_unique<VarDeclStmt>(name.value, std::move(value));
}
std::unique_ptr<Stmt> Parser::parseAssignment() {
    auto name = consume(TokenType::IDENTIFIER, "Expected variable name"); consume(TokenType::EQUAL, "Expected '=' in assignment"); auto value = parseExpression(); consume(TokenType::SEMICOLON, "Expected ';' after assignment"); return std::make_unique<AssignmentStmt>(name.value, std::move(value));
}
std::unique_ptr<Stmt> Parser::parseFunction() {
    advance(); auto name = consume(TokenType::IDENTIFIER, "Expected function name"); consume(TokenType::LPAREN, "Expected '(' after function name");
    std::vector<std::string> params;
    if (!check(TokenType::RPAREN)) { do { params.push_back(consume(TokenType::IDENTIFIER, "Expected parameter name").value); if (!check(TokenType::COMMA)) break; advance(); } while (true); }
    consume(TokenType::RPAREN, "Expected ')' after parameters"); auto body = parseBlock();
    return std::make_unique<FunctionStmt>(name.value, std::move(params), std::move(body));
}
std::unique_ptr<Stmt> Parser::parseWatch() {
    advance();
    auto variable = consume(TokenType::IDENTIFIER, "Expected variable name after 'watch'");
    auto body = parseBlock();
    return std::make_unique<WatchStmt>(variable.value, std::move(body));
}
std::unique_ptr<Stmt> Parser::parseSettle() {
    advance();
    consume(TokenType::IDENTIFIER, "Expected 'max' after 'settle'");
    const auto limit = consume(TokenType::INTEGER, "Expected iteration limit after 'settle max'");
    int maxIterations = std::stoi(limit.value);
    if (maxIterations <= 0) throw std::runtime_error("settle max must be positive");
    auto body = parseBlock();
    std::vector<std::unique_ptr<Stmt>> otherwise;
    if (check(TokenType::ELSE)) { advance(); otherwise = parseBlock(); }
    return std::make_unique<SettleStmt>(maxIterations, std::move(body), std::move(otherwise));
}
std::unique_ptr<Stmt> Parser::parseReturn() {
    advance(); auto value = parseExpression(); consume(TokenType::SEMICOLON, "Expected ';' after return"); return std::make_unique<ReturnStmt>(std::move(value));
}
std::unique_ptr<Stmt> Parser::parseStatement() {
    if (check(TokenType::FUNC)) return parseFunction();
    if (check(TokenType::RETURN)) return parseReturn();
    if (check(TokenType::WATCH)) return parseWatch();
    if (check(TokenType::SETTLE)) return parseSettle();
    if (check(TokenType::PRINT)) return parsePrint();
    if (check(TokenType::IF)) return parseIf();
    if (check(TokenType::WHILE)) return parseWhile();
    if (check(TokenType::VAR)) return parseVarDeclaration();
    if (check(TokenType::IDENTIFIER)) {
        if (current + 1 < tokens.size() && tokens[current + 1].type == TokenType::EQUAL) return parseAssignment();
        auto e = parseExpression(); consume(TokenType::SEMICOLON, "Expected ';' after expression"); return std::make_unique<ExprStmt>(std::move(e));
    }
    throw std::runtime_error("Expected statement");
}
std::vector<std::unique_ptr<Stmt>> Parser::parse() { std::vector<std::unique_ptr<Stmt>> out; while (!check(TokenType::END_OF_FILE)) out.push_back(parseStatement()); return out; }
std::unique_ptr<Expr> Parser::parseExpressionOnly() {
    auto expression = parseExpression();
    consume(TokenType::END_OF_FILE, "Unexpected token in string interpolation");
    return expression;
}
std::vector<std::unique_ptr<Stmt>> Parser::parseBlock() {
    consume(TokenType::LBRACE, "Expected '{'"); std::vector<std::unique_ptr<Stmt>> out;
    while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) out.push_back(parseStatement());
    consume(TokenType::RBRACE, "Expected '}' after block"); return out;
}
std::unique_ptr<Stmt> Parser::parseIf() {
    advance(); consume(TokenType::LPAREN, "Expected '(' after 'if'"); auto condition = parseExpression(); consume(TokenType::RPAREN, "Expected ')' after condition"); auto thenBranch = parseBlock(); std::vector<std::unique_ptr<Stmt>> elseBranch; if (check(TokenType::ELSE)) { advance(); elseBranch = parseBlock(); } return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}
std::unique_ptr<Stmt> Parser::parseWhile() {
    advance(); consume(TokenType::LPAREN, "Expected '(' after 'while'"); auto condition = parseExpression(); consume(TokenType::RPAREN, "Expected ')' after condition"); auto body = parseBlock(); return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}
