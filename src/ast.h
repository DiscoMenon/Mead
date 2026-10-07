#pragma once

#include <memory>
#include <string>
#include <vector>

enum class ValueType {
    INT,
    FLOAT,
    BOOL,
    STRING,
    UNKNOWN
};

struct Expr {
    ValueType type = ValueType::UNKNOWN;
    virtual ~Expr() = default;
};

struct IntegerExpr : Expr {
    int value;

    explicit IntegerExpr(int value)
        : value(value) {
        type = ValueType::INT;
    }
};

struct StringExpr : Expr {
    std::string value;

    explicit StringExpr(const std::string& value)
        : value(value) {
        type = ValueType::STRING;
    }
};

struct FloatExpr : Expr {
    double value;

    explicit FloatExpr(double value)
        : value(value) {
        type = ValueType::FLOAT;
    }
};

struct BoolExpr : Expr {
    bool value;

    explicit BoolExpr(bool value)
        : value(value) {
        type = ValueType::BOOL;
    }
};

struct VariableExpr : Expr {
    std::string name;

    explicit VariableExpr(const std::string& name)
        : name(name) {}
};

struct BinaryExpr : Expr {
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;

    BinaryExpr(
        std::unique_ptr<Expr> left,
        const std::string& op,
        std::unique_ptr<Expr> right
    )
        : left(std::move(left)),
          op(op),
          right(std::move(right)) {}
};

struct Stmt {
    virtual ~Stmt() = default;
};

struct PrintStmt : Stmt {
    std::unique_ptr<Expr> expression;

    explicit PrintStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}
};

struct VarDeclStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> initializer;

    VarDeclStmt(
        const std::string& name,
        std::unique_ptr<Expr> initializer
    )
        : name(name),
          initializer(std::move(initializer)) {}
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;

    std::vector<std::unique_ptr<Stmt>> thenBranch;
    std::vector<std::unique_ptr<Stmt>> elseBranch;

    IfStmt(
        std::unique_ptr<Expr> condition,
        std::vector<std::unique_ptr<Stmt>> thenBranch,
        std::vector<std::unique_ptr<Stmt>> elseBranch
    )
        : condition(std::move(condition)),
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {}
};