
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 4) {
        std::cerr
            << "Usage: meadc <source.mead> [-o output.asm]\n";
        return 1;
    }

    const std::filesystem::path sourcePath = argv[1];
    std::filesystem::path outputPath = sourcePath;

    if (argc == 4) {
        if (std::string(argv[2]) != "-o") {
            std::cerr << "Unknown option: " << argv[2] << '\n';
            return 1;
        }
        outputPath = argv[3];
    } else {
        outputPath.replace_extension(".asm");
    }

    std::ifstream file(sourcePath);
    if (!file) {
        std::cerr << "meadc: cannot open source file: "
                  << sourcePath.string() << '\n';
        return 1;
    }

    const std::string source(
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
        const std::string assembly = generator.generate(statements);

        std::ofstream output(outputPath);
        if (!output) {
            std::cerr << "meadc: cannot create output file: "
                      << outputPath.string() << '\n';
            return 1;
        }

        output << assembly;
        output.close();

        if (!output) {
            std::cerr << "meadc: failed writing output file: "
                      << outputPath.string() << '\n';
            return 1;
        }

        std::cout << "Compilation successful.\n"
                  << "Source: " << sourcePath.string() << '\n'
                  << "Assembly: " << outputPath.string() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "meadc: compilation failed: "
                  << error.what() << '\n';
        return 1;
    }
}
