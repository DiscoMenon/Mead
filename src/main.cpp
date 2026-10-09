
#define NOMINMAX
#include <type_traits>

#include "token.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

#include <windows.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <cctype>

namespace fs = std::filesystem;

// Quote an argument according to Windows command-line parsing rules.
static std::wstring quoteArg(const std::wstring& arg) {
    std::wstring result = L"\"";
    size_t backslashes = 0;

    for (wchar_t ch : arg) {
        if (ch == L'\\') {
            ++backslashes;
        } else if (ch == L'"') {
            result.append(backslashes * 2 + 1, L'\\');
            result += L'"';
            backslashes = 0;
        } else {
            result.append(backslashes, L'\\');
            backslashes = 0;
            result += ch;
        }
    }

    result.append(backslashes * 2, L'\\');
    result += L'"';
    return result;
}

static int runTool(const std::vector<std::wstring>& args) {
    std::wstring commandLine;

    for (const auto& arg : args) {
        if (!commandLine.empty())
            commandLine += L' ';

        commandLine += quoteArg(arg);
    }

    std::vector<wchar_t> mutableCommand(
        commandLine.begin(), commandLine.end()
    );
    mutableCommand.push_back(L'\0');

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startup.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION process{};

    if (!CreateProcessW(
            nullptr,
            mutableCommand.data(),
            nullptr,
            nullptr,
            TRUE,
            0,
            nullptr,
            nullptr,
            &startup,
            &process)) {
        std::wcerr << L"meadc: could not start "
                   << args.front()
                   << L" (Windows error " << GetLastError()
                   << L").\n";
        return -1;
    }

    WaitForSingleObject(process.hProcess, INFINITE);

    DWORD exitCode = 1;
    const BOOL gotExitCode =
        GetExitCodeProcess(process.hProcess, &exitCode);

    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);

    if (!gotExitCode) {
        std::wcerr << L"meadc: could not read tool exit code.\n";
        return -1;
    }

    return static_cast<int>(exitCode);
}

static int fail(const std::string& message) {
    std::cerr << "meadc: " << message << '\n';
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 4) {
        std::cerr
            << "Usage: meadc <source.mead> [-o output.asm|output.exe]\n";
        return 1;
    }

    try {
        const fs::path sourcePath = fs::absolute(argv[1]);

        fs::path outputPath = sourcePath;

        if (argc == 4) {
            if (std::string(argv[2]) != "-o")
                return fail("unknown option: " + std::string(argv[2]));

            outputPath = fs::absolute(argv[3]);
        } else {
            outputPath.replace_extension(".asm");
        }

        std::string extension = outputPath.extension().string();

        for (char& ch : extension)
            ch = static_cast<char>(std::tolower(
                static_cast<unsigned char>(ch)
            ));

        const bool buildExe = extension == ".exe";

        if (!buildExe && extension != ".asm") {
            return fail("output extension must be .asm or .exe");
        }

        fs::path assemblyPath = outputPath;
        fs::path objectPath;

        if (buildExe) {
            assemblyPath.replace_extension(".asm");
            objectPath = outputPath;
            objectPath.replace_extension(".obj");
        }

        std::ifstream file(sourcePath, std::ios::binary);

        if (!file)
            return fail("cannot open source file: " +
                        sourcePath.string());

        const std::string source(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );

        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto statements = parser.parse();

        SemanticAnalyzer analyzer;
        analyzer.analyze(statements);

        CodeGenerator generator;
        const std::string assembly = generator.generate(statements);

        std::ofstream output(
            assemblyPath, std::ios::binary | std::ios::trunc
        );

        if (!output)
            return fail("cannot create assembly file: " +
                        assemblyPath.string());

        output << assembly;
        output.close();

        if (!output)
            return fail("failed writing assembly file: " +
                        assemblyPath.string());

        if (!buildExe) {
            std::cout << "Compilation successful.\n"
                      << "Source: " << sourcePath.string() << '\n'
                      << "Assembly: " << assemblyPath.string() << '\n';
            return 0;
        }

        std::cout << "Assembling...\n";

        const int asmResult = runTool({
            L"ml64.exe",
            L"/c",
            L"/Fo" + objectPath.wstring(),
            assemblyPath.wstring()
        });

        if (asmResult != 0)
            return fail("assembler failed with exit code " +
                        std::to_string(asmResult));

        std::cout << "Linking...\n";

        const int linkResult = runTool({
            L"link.exe",
            L"/SUBSYSTEM:CONSOLE",
            L"/ENTRY:main",
            objectPath.wstring(),
            L"kernel32.lib",
            L"/OUT:" + outputPath.wstring()
        });

        if (linkResult != 0)
            return fail("linker failed with exit code " +
                        std::to_string(linkResult));

        std::cout << "Compilation successful.\n"
                  << "Source: " << sourcePath.string() << '\n'
                  << "Executable: " << outputPath.string() << '\n';

        return 0;

    } catch (const std::exception& error) {
        return fail(std::string("compilation failed: ") + error.what());
    }
}
