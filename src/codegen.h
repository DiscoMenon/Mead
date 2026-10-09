#pragma once

#include "ast.h"
#include <string>
#include <vector>

class CodeGenerator {
public:
    std::string generate(
        const std::vector<std::unique_ptr<Stmt>>& statements
    );
};