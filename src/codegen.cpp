#include "codegen.h"
#include <functional>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <cctype>

std::string CodeGenerator::generate(const std::vector<std::unique_ptr<Stmt>>& statements) {
    std::ostringstream data, code;
    data << ".data\nmead_written DWORD 0\n";
    int labelCount=0, stringCount=0;
    auto fnLabel=[](const std::string& n){ return std::string("mead_fn_")+n; };

    std::function<void(const std::vector<std::unique_ptr<Stmt>>&, const std::string&, const std::vector<std::string>&, bool)> emitFunction;
    emitFunction = [&](const std::vector<std::unique_ptr<Stmt>>& body, const std::string& functionName, const std::vector<std::string>& params, bool isMain) {
        std::unordered_map<std::string,int> slots; int slotCount=0;
        std::vector<const WatchStmt*> watchers;
        std::unordered_map<const SettleStmt*, std::string> settleIds;
        for(const auto& p:params) slots[p]=++slotCount;
        std::function<void(const Stmt*)> collect=[&](const Stmt* s){
            if(auto v=dynamic_cast<const VarDeclStmt*>(s)){ if(!slots.count(v->name)) slots[v->name]=++slotCount; }
            else if(auto i=dynamic_cast<const IfStmt*>(s)){ for(auto& x:i->thenBranch) collect(x.get()); for(auto& x:i->elseBranch) collect(x.get()); }
            else if(auto w=dynamic_cast<const WhileStmt*>(s)){ for(auto& x:w->body) collect(x.get()); }
            else if(auto st=dynamic_cast<const SettleStmt*>(s)){ std::string suffix=std::to_string(labelCount++); settleIds[st]=suffix; slots["__settle_flag_"+suffix]=++slotCount; slots["__settle_count_"+suffix]=++slotCount; for(auto& x:st->body) collect(x.get()); for(auto& x:st->elseBranch) collect(x.get()); }
            else if(auto w=dynamic_cast<const WatchStmt*>(s)){
                const std::string snapshot = "__watch_snapshot_" + std::to_string(watchers.size());
                if (!slots.count(snapshot)) slots[snapshot] = ++slotCount;
                watchers.push_back(w);
                for(auto& x:w->body) collect(x.get());
            }
        };
        for(const auto& s:body) if(!dynamic_cast<const FunctionStmt*>(s.get())) collect(s.get());
        int frameSize = (40 + slotCount * 8 + 15) & ~15;
        std::string endLabel=functionName+"_return";
        code << (isMain?"main":fnLabel(functionName)) << " PROC\n    push rbp\n    mov rbp, rsp\n    sub rsp, " << frameSize << "\n";
        if(!isMain){ const char* regs[]={"rcx","rdx","r8","r9"}; for(size_t i=0;i<params.size();++i) code << "    mov QWORD PTR [rbp-" << slots[params[i]]*8 << "], " << regs[i] << "\n"; }
        auto slot=[&](const std::string& n)->int { auto it=slots.find(n); if(it==slots.end()) throw std::runtime_error("Unknown variable: "+n); return it->second*8; };
        std::function<void(const Expr*)> emitExpr;
        std::function<void(const Stmt*)> emitStmt;
        std::function<void(const std::string&)> emitWatchTriggers;
        bool emittingWatchBody = false;
        std::vector<std::string> activeSettleFlags;
        int stackDepth = 0;
        auto emitWriteBytes = [&](const std::string& text) {
            if (text.empty()) return;
            std::string label = "mead_string_" + std::to_string(stringCount++);
            data << label << " BYTE ";
            for (size_t i = 0; i < text.size(); ++i) {
                if (i) data << ", ";
                data << static_cast<unsigned int>(static_cast<unsigned char>(text[i]));
            }
            data << "\n";
            code << "    mov ecx, -11\n    call GetStdHandle\n"
                 << "    mov rcx, rax\n    lea rdx, " << label << "\n"
                 << "    mov r8d, " << text.size() << "\n"
                 << "    lea r9, mead_written\n"
                 << "    mov QWORD PTR [rsp+20h], 0\n    call WriteFile\n";
        };
        emitExpr=[&](const Expr* e){
            if(auto n=dynamic_cast<const IntegerExpr*>(e)) code<<"    mov rax, "<<n->value<<"\n";
            else if(auto b=dynamic_cast<const BoolExpr*>(e)) code<<"    mov rax, "<<(b->value?1:0)<<"\n";
            else if(auto u=dynamic_cast<const UnitExpr*>(e)) { emitExpr(u->value.get()); if (u->scale != 1) code << "    imul rax, " << u->scale << "\n"; }
            else if(auto v=dynamic_cast<const VariableExpr*>(e)) code<<"    mov rax, QWORD PTR [rbp-"<<slot(v->name)<<"]\n";
            else if(auto call=dynamic_cast<const CallExpr*>(e)){
                if (call->name == "panic") {
                    auto message = dynamic_cast<const StringExpr*>(call->arguments[0].get());
                    if (!message) throw std::runtime_error("panic expects a string literal");
                    emitWriteBytes(message->value + "\r\n");
                    code << "    mov ecx, 1\n    call ExitProcess\n";
                    return;
                }
                if(call->arguments.size()>4) throw std::runtime_error("Mead functions currently support at most 4 parameters");
                for(const auto& a:call->arguments){ emitExpr(a.get()); code<<"    push rax\n"; ++stackDepth; }
                const char* regs[]={"rcx","rdx","r8","r9"};
                for(size_t i=call->arguments.size();i>0;--i) { code<<"    pop "<<regs[i-1]<<"\n"; --stackDepth; }
                int callSpace = (stackDepth % 2 == 0) ? 32 : 40;
                code<<"    sub rsp, "<<callSpace<<"\n    call "<<fnLabel(call->name)<<"\n    add rsp, "<<callSpace<<"\n";
            }
            else if(auto b=dynamic_cast<const BinaryExpr*>(e)){
                emitExpr(b->left.get()); code<<"    push rax\n"; ++stackDepth; emitExpr(b->right.get()); code<<"    pop r10\n"; --stackDepth; const auto& op=b->op;
                if(op=="+") code<<"    add r10, rax\n    mov rax, r10\n";
                else if(op=="-") code<<"    sub r10, rax\n    mov rax, r10\n";
                else if(op=="*") code<<"    imul rax, r10\n";
                else if(op=="/"||op=="%") { code<<"    mov r11, rax\n    mov rax, r10\n    cqo\n    idiv r11\n"; if(op=="%") code<<"    mov rax, rdx\n"; }
                else if(op=="==") code<<"    cmp r10, rax\n    sete al\n    movzx rax, al\n";
                else if(op=="!=") code<<"    cmp r10, rax\n    setne al\n    movzx rax, al\n";
                else if(op=="<") code<<"    cmp r10, rax\n    setl al\n    movzx rax, al\n";
                else if(op=="<=") code<<"    cmp r10, rax\n    setle al\n    movzx rax, al\n";
                else if(op==">") code<<"    cmp r10, rax\n    setg al\n    movzx rax, al\n";
                else if(op==">=") code<<"    cmp r10, rax\n    setge al\n    movzx rax, al\n";
                else if(op=="&&") code<<"    and rax, r10\n";
                else if(op=="||") code<<"    or rax, r10\n";
                else throw std::runtime_error("Unsupported operator: "+op);
            } else if(dynamic_cast<const InterpolatedStringExpr*>(e)) {
                throw std::runtime_error("Interpolated strings are currently supported in print statements only");
            } else if(dynamic_cast<const FloatExpr*>(e)||dynamic_cast<const StringExpr*>(e)) throw std::runtime_error("Float/string expressions are not supported in generated arithmetic or calls yet.");
            else throw std::runtime_error("Unsupported expression in code generation");
        };
        emitStmt=[&](const Stmt* s){
            if(auto f=dynamic_cast<const FunctionStmt*>(s)){ (void)f; return; }
            if(auto r=dynamic_cast<const ReturnStmt*>(s)){ emitExpr(r->value.get()); code<<"    jmp "<<endLabel<<"\n"; }
            else if(auto e=dynamic_cast<const ExprStmt*>(s)) emitExpr(e->expression.get());
            else if(auto v=dynamic_cast<const VarDeclStmt*>(s)){ emitExpr(v->initializer.get()); code<<"    mov QWORD PTR [rbp-"<<slot(v->name)<<"], rax\n"; }
            else if(auto a=dynamic_cast<const AssignmentStmt*>(s)){
                emitExpr(a->value.get());
                if (!activeSettleFlags.empty()) {
                    const int id = labelCount++;
                    const std::string same = "mead_settle_same_" + std::to_string(id);
                    code << "    cmp rax, QWORD PTR [rbp-" << slot(a->name) << "]\n    je " << same << "\n"
                         << "    mov QWORD PTR [rbp-" << slot(a->name) << "], rax\n"
                         << "    mov QWORD PTR [rbp-" << slot(activeSettleFlags.back()) << "], 1\n"
                         << same << ":\n";
                } else {
                    code << "    mov QWORD PTR [rbp-" << slot(a->name) << "], rax\n";
                    if (!emittingWatchBody && emitWatchTriggers) emitWatchTriggers(a->name);
                }
            }
            else if(auto watch=dynamic_cast<const WatchStmt*>(s)){
                auto it = std::find(watchers.begin(), watchers.end(), watch);
                const int index = static_cast<int>(std::distance(watchers.begin(), it));
                code << "    mov rax, QWORD PTR [rbp-" << slot(watch->variable) << "]\n"
                     << "    mov QWORD PTR [rbp-" << slot("__watch_snapshot_" + std::to_string(index)) << "], rax\n";
            }
            else if(auto st=dynamic_cast<const SettleStmt*>(s)) {
                const std::string suffix = settleIds.at(st);
                const std::string flag = "__settle_flag_" + suffix, count = "__settle_count_" + suffix;
                const std::string start = "mead_settle_start_" + suffix, success = "mead_settle_success_" + suffix, failure = "mead_settle_failure_" + suffix, done = "mead_settle_done_" + suffix;
                code << "    mov QWORD PTR [rbp-" << slot(count) << "], " << st->maxIterations << "\n" << start << ":\n"
                     << "    mov QWORD PTR [rbp-" << slot(flag) << "], 0\n";
                activeSettleFlags.push_back(flag);
                for (const auto& child : st->body) emitStmt(child.get());
                activeSettleFlags.pop_back();
                code << "    cmp QWORD PTR [rbp-" << slot(flag) << "], 0\n    je " << success << "\n"
                     << "    sub QWORD PTR [rbp-" << slot(count) << "], 1\n"
                     << "    cmp QWORD PTR [rbp-" << slot(count) << "], 0\n    je " << failure << "\n"
                     << "    jmp " << start << "\n" << success << ":\n    jmp " << done << "\n" << failure << ":\n";
                for (const auto& child : st->elseBranch) emitStmt(child.get());
                code << done << ":\n";
            }
            else if(auto p=dynamic_cast<const PrintStmt*>(s)){
                if(auto str=dynamic_cast<const StringExpr*>(p->expression.get())) {
                    emitWriteBytes(str->value + "\r\n");
                } else if (auto interpolated = dynamic_cast<const InterpolatedStringExpr*>(p->expression.get())) {
                    for (const auto& part : interpolated->parts) {
                        if (!part.expression) {
                            emitWriteBytes(part.text);
                            continue;
                        }
                        emitExpr(part.expression.get());
                        if (part.expression->type == ValueType::INT) {
                            code << "    mov ecx, eax\n    call mead_print_int_inline\n";
                        } else if (part.expression->type == ValueType::BOOL) {
                            int id = labelCount++;
                            std::string falseLabel = "mead_interp_false_" + std::to_string(id);
                            std::string endLabel = "mead_interp_end_" + std::to_string(id);
                            code << "    test rax, rax\n    jz " << falseLabel << "\n";
                            emitWriteBytes("true");
                            code << "    jmp " << endLabel << "\n" << falseLabel << ":\n";
                            emitWriteBytes("false");
                            code << endLabel << ":\n";
                        } else {
                            throw std::runtime_error("String interpolation currently supports integer and boolean expressions");
                        }
                    }
                    emitWriteBytes("\r\n");
                } else {
                    emitExpr(p->expression.get());
                    code << "    mov ecx, eax\n    call mead_print_int\n";
                }
            }
            else if(auto i=dynamic_cast<const IfStmt*>(s)){ int id=labelCount++; auto els="mead_else_"+std::to_string(id), end="mead_if_end_"+std::to_string(id); emitExpr(i->condition.get()); code<<"    test rax, rax\n    jz "<<els<<"\n"; for(auto& x:i->thenBranch) emitStmt(x.get()); if(!i->elseBranch.empty()) code<<"    jmp "<<end<<"\n"; code<<els<<":\n"; for(auto& x:i->elseBranch) emitStmt(x.get()); if(!i->elseBranch.empty()) code<<end<<":\n"; }
            else if(auto w=dynamic_cast<const WhileStmt*>(s)){ int id=labelCount++; auto start="mead_while_"+std::to_string(id), end="mead_while_end_"+std::to_string(id); code<<start<<":\n"; emitExpr(w->condition.get()); code<<"    test rax, rax\n    jz "<<end<<"\n"; for(auto& x:w->body) emitStmt(x.get()); code<<"    jmp "<<start<<"\n"<<end<<":\n"; }
            else throw std::runtime_error("Unsupported statement in code generation");
        };
        emitWatchTriggers = [&](const std::string& changedName) {
            for (size_t i = 0; i < watchers.size(); ++i) {
                const auto* watch = watchers[i];
                if (watch->variable != changedName) continue;
                const int id = labelCount++;
                const std::string done = "mead_watch_done_" + std::to_string(id);
                const std::string snapshotName = "__watch_snapshot_" + std::to_string(i);
                code << "    mov rax, QWORD PTR [rbp-" << slot(watch->variable) << "]\n"
                     << "    cmp rax, QWORD PTR [rbp-" << slot(snapshotName) << "]\n"
                     << "    je " << done << "\n"
                     << "    mov QWORD PTR [rbp-" << slot(snapshotName) << "], rax\n";
                const bool previous = emittingWatchBody;
                emittingWatchBody = true;
                for (const auto& child : watch->body) emitStmt(child.get());
                emittingWatchBody = previous;
                code << done << ":\n";
            }
        };
        for(const auto& s:body) if(!dynamic_cast<const FunctionStmt*>(s.get())) emitStmt(s.get());
        code<<endLabel<<":\n";
        if(isMain) code<<"    xor ecx, ecx\n    call ExitProcess\n";
        code<<"    add rsp, "<<frameSize<<"\n    pop rbp\n    ret\n";
        if(isMain) code<<"main ENDP\n\n"; else code<<fnLabel(functionName)<<" ENDP\n\n";
    };

    // Main code first; function bodies are emitted after main and before helpers.
    std::vector<std::unique_ptr<Stmt>> topLevel;
    for(const auto& s:statements) if(!dynamic_cast<const FunctionStmt*>(s.get())) { /* emitted separately below */ }
    emitFunction(statements,"main",{},true);
    for(const auto& s:statements) if(auto f=dynamic_cast<const FunctionStmt*>(s.get())) {
        // Reuse function emitter; it emits a PROC and a matching ENDP below via a small post-fix.
        emitFunction(f->body,f->name,f->parameters,false);
    }

    code<<R"ASM(
mead_print_int_inline PROC
    sub rsp, 58h
    movsxd rax, ecx
    xor r9d, r9d
    test rax, rax
    jns mead_pii_positive
    neg rax
    mov r9d, 1
mead_pii_positive:
    lea r10, [rsp+3Dh]
    mov r11, 10
    xor r8d, r8d
    test rax, rax
    jnz mead_pii_loop
    mov BYTE PTR [r10], 30h
    dec r10
    inc r8d
    jmp mead_pii_digits_done
mead_pii_loop:
    xor edx, edx
    div r11
    add dl, 30h
    mov BYTE PTR [r10], dl
    dec r10
    inc r8d
    test rax, rax
    jnz mead_pii_loop
mead_pii_digits_done:
    test r9d, r9d
    jz mead_pii_ready
    mov BYTE PTR [r10], 2Dh
    dec r10
    inc r8d
mead_pii_ready:
    lea rdx, [r10+1]
    mov QWORD PTR [rsp+40h], rdx
    mov DWORD PTR [rsp+48h], r8d
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    mov rdx, QWORD PTR [rsp+40h]
    mov r8d, DWORD PTR [rsp+48h]
    lea r9, [rsp+50h]
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    add rsp, 58h
    ret
mead_print_int_inline ENDP

mead_print_int PROC
    sub rsp, 58h
    movsxd rax, ecx
    xor r9d, r9d
    test rax, rax
    jns mead_pi_positive
    neg rax
    mov r9d, 1
mead_pi_positive:
    mov BYTE PTR [rsp+3Eh], 13
    mov BYTE PTR [rsp+3Fh], 10
    lea r10, [rsp+3Dh]
    mov r11, 10
    mov r8d, 2
    test rax, rax
    jnz mead_pi_loop
    mov BYTE PTR [r10], 30h
    dec r10
    inc r8d
    jmp mead_pi_digits_done
mead_pi_loop:
    xor edx, edx
    div r11
    add dl, 30h
    mov BYTE PTR [r10], dl
    dec r10
    inc r8d
    test rax, rax
    jnz mead_pi_loop
mead_pi_digits_done:
    test r9d, r9d
    jz mead_pi_ready
    mov BYTE PTR [r10], 2Dh
    dec r10
    inc r8d
mead_pi_ready:
    lea rdx, [r10+1]
    mov QWORD PTR [rsp+40h], rdx
    mov DWORD PTR [rsp+48h], r8d
    mov ecx, -11
    call GetStdHandle
    mov rcx, rax
    mov rdx, QWORD PTR [rsp+40h]
    mov r8d, DWORD PTR [rsp+48h]
    lea r9, [rsp+50h]
    mov QWORD PTR [rsp+20h], 0
    call WriteFile
    add rsp, 58h
    ret
mead_print_int ENDP
)ASM";
    code<<"END\n";

    return "EXTERN GetStdHandle:PROC\nEXTERN WriteFile:PROC\nEXTERN ExitProcess:PROC\n\n"+data.str()+"\n.code\n"+code.str();
}
