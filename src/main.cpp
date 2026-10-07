#include "lexer.h"
#include "parser.h"
#include "semantic.h"

#include <fstream>
#include <iostream>

int main() {

    std::ifstream file("../examples/hello.mead");

    if (!file) {
        std::cerr << "Could not open file\n";
        return 1;
    }

    std::string source(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    try {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        auto statements = parser.parse();

        SemanticAnalyzer analyzer;
        analyzer.analyze(statements);

        std::cout << "Parsed "
                  << statements.size()
                  << " statement(s) successfully.\n";
    }
    catch (const std::runtime_error& error) {
        std::cerr << "Parse error: "
                  << error.what()
                  << "\n";
        return 1;
    }

    return 0;
}