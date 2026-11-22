# Mead

### A statically typed language with a direct x64 assembly compiler

Mead is a small statically typed programming language with a compiler written in C++. The compiler generates x64 assembly directly, which is assembled and linked with Microsoft's tools to produce standalone Windows executables. The executables do not use the C runtime.

Mead has three uncommon features: **dimensional arithmetic** (adding a length to a time is a compile-time error), **`watch`** statements (run a block when a variable changes) and **`settle`** statements (repeat a block until nothing changes).

---

## Table of Contents

- [Language Features](#language-features)
- [Signature Features](#signature-features)
- [Compiler Architecture](#compiler-architecture)
- [Technology Stack](#technology-stack)
- [Building and Running](#building-and-running)
- [Testing](#testing)
- [Example](#example)
- [Language Quick Reference](#language-quick-reference)
- [Project Structure](#project-structure)
- [Not Yet Implemented](#not-yet-implemented)

---

## Language Features

### 1. Static Typing and Type Inference

Variable types are inferred from their initial values. Function parameter types are inferred from how the function is called, and return types from its `return` statements. Supported types are integers, booleans and strings.

```mead
var age = 21;
var name = "Mead";
var active = true;
```

### 2. Variables and Assignment

```mead
var score = 100;
score = score + 25;
print(score);
```

### 3. Arithmetic and Comparisons

```mead
var total = 10 + 20 * 3;
var passed = total >= 50;
```

`if` and `while` conditions must be boolean expressions.

### 4. Conditional Statements

```mead
var marks = 85;

if (marks >= 90) {
    print("Excellent");
} else {
    print("Keep improving");
}
```

### 5. Loops

```mead
var i = 0;

while (i < 5) {
    print(i);
    i = i + 1;
}
```

### 6. Functions

Functions take up to four parameters. No types are written; they are inferred from the call.

```mead
func square(x) {
    return x * x;
}

var result = square(5);
print(result);
```

If a function is called with two different parameter types, the compiler reports an error.

### 7. Strings and Interpolation

Any expression can be placed inside `${ }` in a string.

```mead
var name = "Mead";
print("Welcome to ${name}!");
```

---

## Signature Features

### Unit-Aware Types

Numbers can carry units using a suffix: `m`, `km` (length) and `s`, `min`, `h` (time). All values are converted to base units (metres and seconds) at compile time.

```mead
var distance = 2.km + 500.m;
var duration = 30.s + 2.min;
print(distance);
print(duration);
```

This prints `2500` and `150`.

Addition, subtraction and comparison require both sides to have the same dimension. Multiplication and division combine dimensions. Mixing dimensions is rejected at compile time:

```mead
var d = 5.m + 2.s;
```

```text
meadc: compilation failed: Cannot add or subtract incompatible dimensions
```

No assembly is produced when this error occurs.

### Reactive Variables with `watch`

A `watch` block runs when the variable it watches changes value.

```mead
var price = 100;

watch price {
    print("Price changed: ${price}");
}

price = 120;
price = 120;
```

The block runs once. The second assignment has the same value, so nothing changes and the block does not run. The compiler keeps a hidden copy of the variable and compares against it after each assignment.

### The `settle` Statement

`settle` repeats a block until a complete pass changes no values. A maximum number of passes is given with `max`, and the `else` block runs if the limit is reached first.

```mead
var n = 0;

settle max 10 {
    if (n < 3) {
        n = n + 1;
    }
} else {
    panic("settle did not finish");
}

print(n);
```

This prints `3`.

---

## Compiler Architecture

```text
          Mead Source Code
                  |
                  v
           Lexical Analysis
                  |
                  v
                Parsing
                  |
                  v
        Abstract Syntax Tree (AST)
                  |
                  v
          Semantic Analysis
          (types + dimensions)
                  |
                  v
            Code Generation
                  |
                  v
           x64 Assembly (MASM)
                  |
                  v
       Microsoft Assembler (ml64)
                  |
                  v
          Microsoft Linker
                  |
                  v
           Windows Executable
```

1. **Lexical analysis** converts source text into tokens. Unit suffixes such as `.km` are part of number tokens. When `${` is found in a string, the contents are lexed and parsed as a normal expression.
2. **Parsing** uses recursive descent and builds the AST. Unit, watch and settle have their own node types (`UnitExpr`, `WatchStmt`, `SettleStmt`) and are not rewritten into loops or conditions.
3. **Semantic analysis** adds type and dimension information to each expression, infers parameter and return types, and reports errors such as undefined variables, type changes on assignment and mismatched units.
4. **Code generation** translates the annotated AST directly into x64 assembly, with no intermediate representation. It follows the Win64 calling convention: the first four arguments go in `rcx`, `rdx`, `r8` and `r9`, with 32 bytes of shadow space.
5. **Assembling and linking** are done by `ml64.exe` and `link.exe`, run automatically by the compiler driver.

The generated program includes a minimal runtime. Output uses `WriteFile`, and the program ends with `ExitProcess`. Only `kernel32.lib` is linked, so the C runtime is not used.

---

## Technology Stack

- C++17 for the compiler
- CMake for the build
- x64 assembly (MASM syntax) as compiler output
- Microsoft `ml64.exe` and `link.exe` for assembling and linking
- Python 3 for the test runner
- Windows as the target platform

---

## Building and Running

### Requirements

- Windows 10 or 11 (64-bit)
- Visual Studio 2022 with the "Desktop development with C++" workload (provides MSVC, `ml64.exe`, `link.exe` and the Windows SDK)
- CMake 3.16 or newer
- Python 3 (only needed for the tests)

Run all commands from the **x64 Native Tools Command Prompt for VS 2022**. This puts the assembler and linker on your path.

### 1. Build the Compiler

```bat
git clone https://github.com/DiscoMenon/Mead.git
cd Mead
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

This creates `build\Debug\meadc.exe`.

### 2. Compile a Mead Program

```bat
build\Debug\meadc.exe examples\hello.mead -o build\hello.exe
```

The compiler generates `build\hello.asm`, then runs `ml64.exe` and `link.exe`, and prints each step.

To stop after generating assembly, give the output an `.asm` extension:

```bat
build\Debug\meadc.exe examples\hello.mead -o build\hello.asm
```

### 3. Run the Executable

```bat
build\hello.exe
```

---

## Testing

```bat
ctest --test-dir build -C Debug --output-on-failure
```

The test script compiles and runs 8 valid programs and checks their output. It also checks that 3 invalid programs (undefined variable, assignment that changes a variable's type, incompatible units) fail to compile with the expected error. The tests cover arithmetic, control flow, functions, strings and interpolation, units, `watch` and `settle`.

To list each program's result, add `-V`:

```bat
ctest --test-dir build -C Debug -V
```

---

## Example

```mead
var x = 10;
var y = 20;
var result = x + y;

if (result > 25) {
    print("Result exceeds 25");
} else {
    print("Result is 25 or less");
}

print(result);
```

Output:

```text
Result exceeds 25
30
```

`examples\hello.mead` uses every feature: arithmetic, conditionals, loops, functions, interpolation, units, `watch` and `settle`.

---

## Language Quick Reference

```text
Variable      var x = 10;
Assignment    x = x + 1;
Output        print(x);   or   print("Total: ${x}");
Condition     if (x > 5) { ... } else { ... }
Loop          while (x < 10) { ... }
Function      func add(a, b) { return a + b; }
Units         2.km + 500.m   (units: m, km, s, min, h)
Watch         watch x { ... }
Settle        settle max 10 { ... } else { ... }
```

Every statement ends with a semicolon.

---

## Project Structure

```text
Mead/
├── src/
│   ├── main.cpp        compiler driver (runs the pipeline, ml64 and link)
│   ├── lexer.cpp/.h    tokens, unit suffixes, string interpolation
│   ├── parser.cpp/.h   recursive-descent parser
│   ├── ast.h           AST nodes
│   ├── token.h         token definitions
│   ├── semantic.cpp/.h type inference and dimension checks
│   └── codegen.cpp/.h  x64 assembly generation
├── examples/
│   └── hello.mead
├── tests/
│   ├── run_tests.py
│   └── *.mead          valid and invalid test programs
├── CMakeLists.txt
└── README.md
```

---

## Not Yet Implemented

- Floating-point numbers and other numeric types
- Arrays and `for` loops
- `Result` error handling with `match`
- Dependent watching (watchers on derived variables)
- Short-circuit evaluation for `&&` and `||`
- Optimisations such as constant folding and register allocation
- Line and column numbers in error messages