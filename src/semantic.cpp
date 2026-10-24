#include "semantic.h"
#include <stdexcept>

void SemanticAnalyzer::analyzeExpression(const Expr* expression) {
    if (dynamic_cast<const FloatExpr*>(expression) || dynamic_cast<const IntegerExpr*>(expression) || dynamic_cast<const BoolExpr*>(expression) || dynamic_cast<const StringExpr*>(expression)) return;
    if (auto interpolated = dynamic_cast<const InterpolatedStringExpr*>(expression)) {
        for (const auto& part : interpolated->parts) {
            if (!part.expression) continue;
            analyzeExpression(part.expression.get());
            if (part.expression->type != ValueType::INT && part.expression->type != ValueType::BOOL && part.expression->type != ValueType::UNKNOWN)
                throw std::runtime_error("String interpolation currently supports integer and boolean expressions");
        }
        const_cast<InterpolatedStringExpr*>(interpolated)->type = ValueType::STRING;
        return;
    }
    if (auto v = dynamic_cast<const VariableExpr*>(expression)) {
        auto it = variables.find(v->name); if (it == variables.end()) throw std::runtime_error("Undefined variable: " + v->name);
        const_cast<VariableExpr*>(v)->type = it->second; return;
    }
    if (auto call = dynamic_cast<const CallExpr*>(expression)) {
        auto it = functions.find(call->name); if (it == functions.end()) throw std::runtime_error("Undefined function: " + call->name);
        if (call->arguments.size() != it->second.parameterTypes.size()) throw std::runtime_error("Wrong number of arguments for: " + call->name);
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            analyzeExpression(call->arguments[i].get());
            auto actual = call->arguments[i]->type; auto& expected = it->second.parameterTypes[i];
            if (expected == ValueType::UNKNOWN) expected = actual;
            else if (actual != ValueType::UNKNOWN && actual != expected) throw std::runtime_error("Argument type mismatch in call to: " + call->name);
        }
        const_cast<CallExpr*>(call)->type = it->second.returnType; return;
    }
    if (auto b = dynamic_cast<const BinaryExpr*>(expression)) {
        analyzeExpression(b->left.get()); analyzeExpression(b->right.get()); auto l=b->left->type, r=b->right->type;
        if (b->op=="&&" || b->op=="||") { if(l!=ValueType::BOOL || r!=ValueType::BOOL) throw std::runtime_error("Logical operators require boolean operands"); const_cast<BinaryExpr*>(b)->type=ValueType::BOOL; return; }
        if (b->op=="==" || b->op=="!=" || b->op=="<" || b->op=="<=" || b->op==">" || b->op==">=") { bool numeric=(l==ValueType::INT||l==ValueType::FLOAT)&&(r==ValueType::INT||r==ValueType::FLOAT); bool same=l==r && l!=ValueType::UNKNOWN; if(!numeric && !((b->op=="=="||b->op=="!=")&&same)) throw std::runtime_error("Invalid types for comparison"); const_cast<BinaryExpr*>(b)->type=ValueType::BOOL; return; }
        if (l==ValueType::INT && r==ValueType::INT) { const_cast<BinaryExpr*>(b)->type=ValueType::INT; return; }
        if ((l==ValueType::INT||l==ValueType::FLOAT)&&(r==ValueType::INT||r==ValueType::FLOAT)) { const_cast<BinaryExpr*>(b)->type=ValueType::FLOAT; return; }
        throw std::runtime_error("Incompatible or uninferred types in binary expression");
    }
    throw std::runtime_error("Unsupported expression in semantic analysis");
}
void SemanticAnalyzer::analyzeStatement(const Stmt* statement) {
    if (auto f=dynamic_cast<const FunctionStmt*>(statement)) {
        auto savedVars=variables; auto savedInside=insideFunction; auto savedReturn=currentReturnType; auto savedName=currentFunction;
        variables.clear(); insideFunction=true; currentFunction=f->name; currentReturnType=ValueType::UNKNOWN;
        auto fit=functions.find(f->name);
        for(size_t i=0;i<f->parameters.size();++i) variables[f->parameters[i]]=fit->second.parameterTypes[i];
        for(const auto& s:f->body) analyzeStatement(s.get());
        fit->second.returnType=currentReturnType;
        variables=std::move(savedVars); insideFunction=savedInside; currentReturnType=savedReturn; currentFunction=savedName; return;
    }
    if(auto r=dynamic_cast<const ReturnStmt*>(statement)) { if(!insideFunction) throw std::runtime_error("'return' outside a function"); analyzeExpression(r->value.get()); if(currentReturnType==ValueType::UNKNOWN) currentReturnType=r->value->type; else if(currentReturnType!=r->value->type) throw std::runtime_error("Function has inconsistent return types: "+currentFunction); return; }
    if(auto v=dynamic_cast<const VarDeclStmt*>(statement)) { analyzeExpression(v->initializer.get()); variables[v->name]=v->initializer->type; return; }
    if(auto a=dynamic_cast<const AssignmentStmt*>(statement)) { auto it=variables.find(a->name); if(it==variables.end()) throw std::runtime_error("Assignment to undefined variable: "+a->name); analyzeExpression(a->value.get()); if(it->second!=ValueType::UNKNOWN && a->value->type!=it->second) throw std::runtime_error("Assignment changes variable type: "+a->name); it->second=a->value->type; return; }
    if (auto watch = dynamic_cast<const WatchStmt*>(statement)) {
        if (insideFunction) throw std::runtime_error("'watch' is currently only supported at top level");
        if (!variables.count(watch->variable))
            throw std::runtime_error("Cannot watch undefined variable: " + watch->variable);
        auto savedVariables = variables;
        for (const auto& child : watch->body) analyzeStatement(child.get());
        variables = std::move(savedVariables);
        return;
    }
    if(auto p=dynamic_cast<const PrintStmt*>(statement)) { analyzeExpression(p->expression.get()); return; }
    if(auto e=dynamic_cast<const ExprStmt*>(statement)) { analyzeExpression(e->expression.get()); return; }
    if(auto i=dynamic_cast<const IfStmt*>(statement)) { analyzeExpression(i->condition.get()); if(i->condition->type!=ValueType::BOOL) throw std::runtime_error("If condition must be boolean"); for(auto& s:i->thenBranch) analyzeStatement(s.get()); for(auto& s:i->elseBranch) analyzeStatement(s.get()); return; }
    if(auto w=dynamic_cast<const WhileStmt*>(statement)) { analyzeExpression(w->condition.get()); if(w->condition->type!=ValueType::BOOL) throw std::runtime_error("While condition must be boolean"); for(auto& s:w->body) analyzeStatement(s.get()); }
}
void SemanticAnalyzer::analyze(const std::vector<std::unique_ptr<Stmt>>& statements) {
    functions.clear(); variables.clear();
    for (const auto& s : statements) {
        if (auto f = dynamic_cast<const FunctionStmt*>(s.get())) {
            if (functions.count(f->name)) throw std::runtime_error("Duplicate function: " + f->name);
            FunctionInfo info; info.parameterTypes.resize(f->parameters.size(), ValueType::UNKNOWN);
            functions.emplace(f->name, std::move(info));
        }
    }

    // Iterate so calls to earlier functions can infer the parameters of callers.
    // Each pass rechecks top-level calls, then analyzes functions whose parameter
    // types are known. This supports common nesting such as square(add(2, 3)).
    for (int pass = 0; pass < 4; ++pass) {
        variables.clear();
        for (const auto& s : statements)
            if (!dynamic_cast<const FunctionStmt*>(s.get())) analyzeStatement(s.get());

        bool progress = false;
        for (const auto& s : statements) {
            auto f = dynamic_cast<const FunctionStmt*>(s.get());
            if (!f) continue;
            const auto& info = functions.at(f->name);
            bool ready = true;
            for (auto type : info.parameterTypes) if (type == ValueType::UNKNOWN) ready = false;
            if (!ready) continue;
            ValueType oldReturn = functions.at(f->name).returnType;
            analyzeStatement(f);
            if (functions.at(f->name).returnType != oldReturn) progress = true;
        }
        if (!progress && pass > 0) break;
    }

    // Final top-level validation after return types have been inferred.
    variables.clear();
    for (const auto& s : statements)
        if (!dynamic_cast<const FunctionStmt*>(s.get())) analyzeStatement(s.get());

    for (const auto& entry : functions) {
        for (auto type : entry.second.parameterTypes)
            if (type == ValueType::UNKNOWN)
                throw std::runtime_error("Cannot infer parameter types for function: " + entry.first + "; call it with typed arguments first");
        if (entry.second.returnType == ValueType::UNKNOWN)
            throw std::runtime_error("Cannot infer return type for function: " + entry.first + "; ensure it returns a value");
    }
}
