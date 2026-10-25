#pragma once
#include "ast.h"
#include <string>
#include <unordered_map>
#include <vector>

class SemanticAnalyzer {
private:
    std::unordered_map<std::string, ValueType> variables;
    std::unordered_map<std::string, std::string> dimensions;
    struct FunctionInfo { std::vector<ValueType> parameterTypes; ValueType returnType = ValueType::UNKNOWN; };
    std::unordered_map<std::string, FunctionInfo> functions;
    ValueType currentReturnType = ValueType::UNKNOWN;
    bool insideFunction = false;
    std::string currentFunction;
    void analyzeStatement(const Stmt* statement);
    void analyzeExpression(const Expr* expression);
public:
    void analyze(const std::vector<std::unique_ptr<Stmt>>& statements);
};
