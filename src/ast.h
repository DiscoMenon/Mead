#pragma once

#include <memory>
#include <string>
#include <vector>

enum class ValueType {
    INT,
    FLOAT,
    BOOL,
    STRING,
    UNKNOWN,
    UNIT
};

struct Expr {
    ValueType type = ValueType::UNKNOWN;
    std::string dimension;
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

struct InterpolatedStringPart {
    std::string text;
    std::unique_ptr<Expr> expression;
};

struct InterpolatedStringExpr : Expr {
    std::vector<InterpolatedStringPart> parts;

    explicit InterpolatedStringExpr(std::vector<InterpolatedStringPart> parts)
        : parts(std::move(parts)) {
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

struct UnitExpr : Expr {
    std::unique_ptr<Expr> value;
    std::string unit;
    long long scale;
    UnitExpr(std::unique_ptr<Expr> value, std::string unit, long long scale, std::string dimension)
        : Expr(), value(std::move(value)), unit(std::move(unit)), scale(scale) { this->dimension = std::move(dimension); type = ValueType::UNIT; }
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

struct CallExpr : Expr {
    std::string name;
    std::vector<std::unique_ptr<Expr>> arguments;

    CallExpr(
        const std::string& name,
        std::vector<std::unique_ptr<Expr>> arguments
    )
        : name(name), arguments(std::move(arguments)) {}
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

struct FunctionStmt : Stmt {
    std::string name;
    std::vector<std::string> parameters;
    std::vector<std::unique_ptr<Stmt>> body;

    FunctionStmt(
        const std::string& name,
        std::vector<std::string> parameters,
        std::vector<std::unique_ptr<Stmt>> body
    )
        : name(name),
          parameters(std::move(parameters)),
          body(std::move(body)) {}
};

struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;

    explicit ReturnStmt(std::unique_ptr<Expr> value)
        : value(std::move(value)) {}
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expression;

    explicit ExprStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}
};


struct WatchStmt : Stmt {
    std::string variable;
    std::vector<std::unique_ptr<Stmt>> body;

    WatchStmt(std::string variable, std::vector<std::unique_ptr<Stmt>> body)
        : variable(std::move(variable)), body(std::move(body)) {}
};

struct SettleStmt : Stmt {
    int maxIterations;
    std::vector<std::unique_ptr<Stmt>> body;
    std::vector<std::unique_ptr<Stmt>> elseBranch;
    SettleStmt(int maxIterations, std::vector<std::unique_ptr<Stmt>> body, std::vector<std::unique_ptr<Stmt>> elseBranch)
        : maxIterations(maxIterations), body(std::move(body)), elseBranch(std::move(elseBranch)) {}
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

struct AssignmentStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> value;

    AssignmentStmt(
        const std::string& name,
        std::unique_ptr<Expr> value
    ) : name(name), value(std::move(value)) {}
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

struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;

    WhileStmt(
        std::unique_ptr<Expr> condition,
        std::vector<std::unique_ptr<Stmt>> body
    )
        : condition(std::move(condition)),
          body(std::move(body)) {}
};