#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int main() {
    std::ifstream file("../examples/hello.mead");

    if (!file) {
        std::cerr << "Could not open ../examples/hello.mead\n";
        return 1;
    }

    std::string source(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto statements = parser.parse();

        SemanticAnalyzer analyzer;
        analyzer.analyze(statements);

        CodeGenerator generator;
        std::string assembly = generator.generate(statements);

        std::ofstream output("generated.asm");
        if (!output) {
            std::cerr << "Could not create generated.asm\n";
            return 1;
        }

        output << assembly;
        output.close();

        if (!output) {
            std::cerr << "Failed to write generated.asm\n";
            return 1;
        }

        std::cout << "Compilation successful.\n"
                  << "Generated generated.asm from "
                  << statements.size()
                  << " statement(s).\n";
    }
    catch (const std::exception& error) {
        std::cerr << "Compilation error: "
                  << error.what() << '\n';
        return 1;
    }

    return 0;
}