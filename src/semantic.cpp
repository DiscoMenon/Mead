#include "semantic.h"

#include <stdexcept>

void SemanticAnalyzer::analyzeExpression(
    const Expr* expression
) {
    if (dynamic_cast<const FloatExpr*>(expression)) {
        return;
    }

    if (dynamic_cast<const BoolExpr*>(expression)) {
        return;
    }

    if (auto integer =
            dynamic_cast<const IntegerExpr*>(expression)) {

        return;
    }

    if (auto string =
            dynamic_cast<const StringExpr*>(expression)) {

        return;
    }

    if (auto variable =
            dynamic_cast<const VariableExpr*>(expression)) {

        auto it = variables.find(variable->name);

        if (it == variables.end()) {
            throw std::runtime_error(
                "Undefined variable: " + variable->name
            );
        }

        const_cast<VariableExpr*>(variable)->type = it->second;

        return;
    }

    if (auto binary =
            dynamic_cast<const BinaryExpr*>(expression)) {

        analyzeExpression(binary->left.get());
        analyzeExpression(binary->right.get());

        ValueType leftType = binary->left->type;
        ValueType rightType = binary->right->type;

        // Logical operators require booleans.
        if (binary->op == "&&" || binary->op == "||") {

            if (leftType != ValueType::BOOL ||
                rightType != ValueType::BOOL) {

                throw std::runtime_error(
                    "Logical operators require boolean operands"
                );
            }

            const_cast<BinaryExpr*>(binary)->type =
                ValueType::BOOL;

            return;
        }

        // Comparisons produce bool.
        if (binary->op == "==" ||
            binary->op == "!=" ||
            binary->op == "<" ||
            binary->op == "<=" ||
            binary->op == ">" ||
            binary->op == ">=") {

            if (leftType == ValueType::INT &&
                rightType == ValueType::INT) {

                const_cast<BinaryExpr*>(binary)->type =
                    ValueType::BOOL;

                return;
            }

            if (leftType == ValueType::FLOAT &&
                rightType == ValueType::FLOAT) {

                const_cast<BinaryExpr*>(binary)->type =
                    ValueType::BOOL;

                return;
            }

            if ((leftType == ValueType::INT &&
                rightType == ValueType::FLOAT) ||
                (leftType == ValueType::FLOAT &&
                rightType == ValueType::INT)) {

                const_cast<BinaryExpr*>(binary)->type =
                    ValueType::BOOL;

                return;
            }

            if ((binary->op == "==" || binary->op == "!=") &&
                leftType == rightType) {

                const_cast<BinaryExpr*>(binary)->type =
                    ValueType::BOOL;

                return;
            }

            throw std::runtime_error(
                "Invalid types for comparison"
            );
        }

        // int + int → int
        if (leftType == ValueType::INT &&
            rightType == ValueType::INT) {

            const_cast<BinaryExpr*>(binary)->type =
                ValueType::INT;

            return;
        }

        // float + float → float
        if (leftType == ValueType::FLOAT &&
            rightType == ValueType::FLOAT) {

            const_cast<BinaryExpr*>(binary)->type =
                ValueType::FLOAT;

            return;
        }

        // int + float → float
        if ((leftType == ValueType::INT &&
            rightType == ValueType::FLOAT) ||
            (leftType == ValueType::FLOAT &&
            rightType == ValueType::INT)) {

            const_cast<BinaryExpr*>(binary)->type =
                ValueType::FLOAT;

            return;
        }

        throw std::runtime_error(
            "Incompatible types in binary expression"
        );
    }
}

void SemanticAnalyzer::analyzeStatement(
    const Stmt* statement
) {
    if (auto declaration =
            dynamic_cast<const VarDeclStmt*>(statement)) {

        analyzeExpression(
            declaration->initializer.get()
        );

        ValueType type =
            declaration->initializer->type;

        variables[declaration->name] = type;

        return;
    }

    if (auto print =
            dynamic_cast<const PrintStmt*>(statement)) {

        analyzeExpression(
            print->expression.get()
        );

        return;
    }
}

void SemanticAnalyzer::analyze(
    const std::vector<std::unique_ptr<Stmt>>& statements
) {
    for (const auto& statement : statements) {
        analyzeStatement(statement.get());
    }
}