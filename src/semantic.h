#pragma once

#include "ast.h"

#include <string>
#include <unordered_map>
#include <vector>

class SemanticAnalyzer {
private:
    std::unordered_map<std::string, ValueType> variables;

    void analyzeStatement(const Stmt* statement);
    void analyzeExpression(const Expr* expression);

public:
    void analyze(
        const std::vector<std::unique_ptr<Stmt>>& statements
    );
};