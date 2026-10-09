
#include "codegen.h"

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

std::string CodeGenerator::generate(
    const std::vector<std::unique_ptr<Stmt>>& statements
) {
    std::ostringstream data;
    std::ostringstream code;

    std::unordered_map<std::string, int> slots;
    int slotCount = 0;
    int labelCount = 0;
    int stringCount = 0;

    data << ".data\n"
         << "mead_written DWORD 0\n";

    // Collect variables from all branches and loops.
    std::function<void(const Stmt*)> collect =
        [&](const Stmt* stmt) {
            if (auto v = dynamic_cast<const VarDeclStmt*>(stmt)) {
                if (!slots.count(v->name)) {
                    slots[v->name] = ++slotCount;
                }
            } else if (auto i = dynamic_cast<const IfStmt*>(stmt)) {
                for (const auto& s : i->thenBranch)
                    collect(s.get());
                for (const auto& s : i->elseBranch)
                    collect(s.get());
            } else if (auto w = dynamic_cast<const WhileStmt*>(stmt)) {
                for (const auto& s : w->body)
                    collect(s.get());
            }
        };

    for (const auto& stmt : statements)
        collect(stmt.get());

    // Reserve shadow space, the fifth-argument slot, and variables.
    int frameSize = 40 + slotCount * 8;
    frameSize = (frameSize + 15) & ~15;

    code << ".code\n"
         << "main PROC\n"
         << "    push rbp\n"
         << "    mov rbp, rsp\n"
         << "    sub rsp, " << frameSize << "\n";

    auto slot = [&](const std::string& name) -> int {
        auto it = slots.find(name);

        if (it == slots.end()) {
            throw std::runtime_error("Unknown variable: " + name);
        }

        return it->second * 8;
    };

    std::function<void(const Expr*)> emitExpr;
    std::function<void(const Stmt*)> emitStmt;

    // Expressions leave their result in RAX.
    emitExpr = [&](const Expr* expr) {
        if (auto n = dynamic_cast<const IntegerExpr*>(expr)) {
            code << "    mov rax, " << n->value << "\n";

        } else if (auto boolean =
                       dynamic_cast<const BoolExpr*>(expr)) {
            code << "    mov rax, "
                 << (boolean->value ? 1 : 0) << "\n";

        } else if (auto variable =
                       dynamic_cast<const VariableExpr*>(expr)) {
            code << "    mov rax, QWORD PTR [rbp-"
                 << slot(variable->name) << "]\n";

        } else if (auto binary =
                       dynamic_cast<const BinaryExpr*>(expr)) {
            emitExpr(binary->left.get());
            code << "    push rax\n";

            emitExpr(binary->right.get());
            code << "    pop r10\n";

            const std::string& op = binary->op;

            if (op == "+") {
                code << "    add r10, rax\n"
                     << "    mov rax, r10\n";

            } else if (op == "-") {
                code << "    sub r10, rax\n"
                     << "    mov rax, r10\n";

            } else if (op == "*") {
                code << "    imul rax, r10\n";

            } else if (op == "/" || op == "%") {
                code << "    mov r11, rax\n"
                     << "    mov rax, r10\n"
                     << "    cqo\n"
                     << "    idiv r11\n";

                if (op == "%") {
                    code << "    mov rax, rdx\n";
                }

            } else if (op == "==") {
                code << "    cmp r10, rax\n"
                     << "    sete al\n"
                     << "    movzx rax, al\n";

            } else if (op == "!=") {
                code << "    cmp r10, rax\n"
                     << "    setne al\n"
                     << "    movzx rax, al\n";

            } else if (op == "<") {
                code << "    cmp r10, rax\n"
                     << "    setl al\n"
                     << "    movzx rax, al\n";

            } else if (op == "<=") {
                code << "    cmp r10, rax\n"
                     << "    setle al\n"
                     << "    movzx rax, al\n";

            } else if (op == ">") {
                code << "    cmp r10, rax\n"
                     << "    setg al\n"
                     << "    movzx rax, al\n";

            } else if (op == ">=") {
                code << "    cmp r10, rax\n"
                     << "    setge al\n"
                     << "    movzx rax, al\n";

            } else if (op == "&&") {
                code << "    and rax, r10\n";

            } else if (op == "||") {
                code << "    or rax, r10\n";

            } else {
                throw std::runtime_error(
                    "Unsupported operator: " + op
                );
            }

        } else if (dynamic_cast<const StringExpr*>(expr) ||
                   dynamic_cast<const FloatExpr*>(expr)) {
            throw std::runtime_error(
                "String/float values aren't supported in arithmetic."
            );

        } else {
            throw std::runtime_error("Unsupported expression.");
        }
    };

    emitStmt = [&](const Stmt* stmt) {
        if (auto v = dynamic_cast<const VarDeclStmt*>(stmt)) {
            emitExpr(v->initializer.get());

            code << "    mov QWORD PTR [rbp-"
                 << slot(v->name) << "], rax\n";

        } else if (auto a =
                       dynamic_cast<const AssignmentStmt*>(stmt)) {
            emitExpr(a->value.get());

            code << "    mov QWORD PTR [rbp-"
                 << slot(a->name) << "], rax\n";

        } else if (auto p = dynamic_cast<const PrintStmt*>(stmt)) {
            if (auto s = dynamic_cast<const StringExpr*>(
                    p->expression.get())) {
                const std::string label =
                    "mead_string_" + std::to_string(stringCount++);

                data << label << " BYTE ";

                for (size_t i = 0; i < s->value.size(); ++i) {
                    if (i != 0)
                        data << ", ";

                    data << static_cast<unsigned int>(
                        static_cast<unsigned char>(s->value[i])
                    );
                }

                if (!s->value.empty())
                    data << ", ";

                data << "13, 10\n";

                code << "    mov ecx, -11\n"
                     << "    call GetStdHandle\n"
                     << "    mov rcx, rax\n"
                     << "    lea rdx, " << label << "\n"
                     << "    mov r8d, "
                     << s->value.size() + 2 << "\n"
                     << "    lea r9, mead_written\n"
                     << "    mov QWORD PTR [rsp+20h], 0\n"
                     << "    call WriteFile\n";

            } else {
                emitExpr(p->expression.get());

                code << "    mov ecx, eax\n"
                     << "    call mead_print_int\n";
            }

        } else if (auto i = dynamic_cast<const IfStmt*>(stmt)) {
            const int id = labelCount++;

            const std::string otherwise =
                "mead_else_" + std::to_string(id);
            const std::string end =
                "mead_if_end_" + std::to_string(id);

            emitExpr(i->condition.get());

            code << "    test rax, rax\n"
                 << "    jz " << otherwise << "\n";

            for (const auto& s : i->thenBranch)
                emitStmt(s.get());

            if (!i->elseBranch.empty())
                code << "    jmp " << end << "\n";

            code << otherwise << ":\n";

            for (const auto& s : i->elseBranch)
                emitStmt(s.get());

            if (!i->elseBranch.empty())
                code << end << ":\n";

        } else if (auto w = dynamic_cast<const WhileStmt*>(stmt)) {
            const int id = labelCount++;

            const std::string start =
                "mead_while_" + std::to_string(id);
            const std::string end =
                "mead_while_end_" + std::to_string(id);

            code << start << ":\n";

            emitExpr(w->condition.get());

            code << "    test rax, rax\n"
                 << "    jz " << end << "\n";

            for (const auto& s : w->body)
                emitStmt(s.get());

            code << "    jmp " << start << "\n"
                 << end << ":\n";

        } else {
            throw std::runtime_error("Unsupported statement.");
        }
    };

    for (const auto& stmt : statements)
        emitStmt(stmt.get());

    code << "    xor ecx, ecx\n"
         << "    call ExitProcess\n"
         << "main ENDP\n\n";

    // Print a signed 32-bit integer followed by CR/LF.
    // Digits and sign are built backwards immediately before CR/LF.
    code << R"ASM(
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

    ; Save buffer pointer and length across GetStdHandle.
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

    code << "END\n";

    return
        "EXTERN GetStdHandle:PROC\n"
        "EXTERN WriteFile:PROC\n"
        "EXTERN ExitProcess:PROC\n\n"
        + data.str() + "\n" + code.str();
}
