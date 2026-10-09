#include "codegen.h"

#include <sstream>
#include <stdexcept>
#include <string>

std::string CodeGenerator::generate(
    const std::vector<std::unique_ptr<Stmt>>& statements
) {
    std::ostringstream data;
    std::ostringstream code;
    int index = 0;

    data << ".data\n";
    data << "mead_written DWORD 0\n";

    code << ".code\n"
         << "main PROC\n"
         << "    sub rsp, 28h\n";

    for (const auto& statement : statements) {
        auto printStmt =
            dynamic_cast<const PrintStmt*>(statement.get());

        if (!printStmt) {
            throw std::runtime_error(
                "Backend currently supports print statements only."
            );
        }

        auto stringExpr = dynamic_cast<const StringExpr*>(
            printStmt->expression.get()
        );

        if (!stringExpr) {
            throw std::runtime_error(
                "Backend currently supports print with string literals only."
            );
        }

        const std::string label =
            "mead_string_" + std::to_string(index++);

        // Emit every source byte as a numeric MASM byte.
        data << label << " BYTE ";
        for (size_t i = 0; i < stringExpr->value.size(); ++i) {
            if (i != 0) data << ", ";
            data << static_cast<unsigned int>(
                static_cast<unsigned char>(stringExpr->value[i])
            );
        }

        if (!stringExpr->value.empty()) data << ", ";
        data << "13, 10\n";

        const size_t length = stringExpr->value.size() + 2;

        code << "    mov ecx, -11\n"
             << "    call GetStdHandle\n"
             << "    mov rcx, rax\n"
             << "    lea rdx, " << label << "\n"
             << "    mov r8d, " << length << "\n"
             << "    lea r9, mead_written\n"
             << "    mov QWORD PTR [rsp+20h], 0\n"
             << "    call WriteFile\n";
    }

    code << "    xor ecx, ecx\n"
         << "    call ExitProcess\n"
         << "main ENDP\n"
         << "END\n";

    return
        "EXTERN GetStdHandle:PROC\n"
        "EXTERN WriteFile:PROC\n"
        "EXTERN ExitProcess:PROC\n\n"
        + data.str() + "\n" + code.str();
}