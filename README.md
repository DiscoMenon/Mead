# Mead

### Python-level simplicity. C-level performance.

Mead is a statically typed programming language built from scratch in C++. It combines readable syntax and type inference with a compiler that generates x64 assembly directly. The generated assembly is processed using Microsoft's assembler and linker to produce standalone Windows executables that do not use the C runtime.

The goal is simple: **make programming easier to write without sacrificing the benefits of compilation.**

---

## Table of Contents

- [Vision](#vision)
- [Language Features](#language-features)
- [Signature Features](#signature-features)
- [Compiler Architecture](#compiler-architecture)
- [Technology Stack](#technology-stack)
- [Building and Running](#building-and-running)
- [Testing](#testing)
- [Example](#example)
- [Language Quick Reference](#language-quick-reference)
- [Project Structure](#project-structure)
- [Planned Features](#planned-features)

---

## Vision

Programming languages often involve a trade-off between development simplicity and execution performance.

Python offers concise syntax, dynamic flexibility, and a low barrier to entry, but programs are interpreted. C and C++ provide fast native execution but require explicit type declarations and manual memory management.

Mead explores a middle ground through:

- **Readable syntax** that keeps programs concise and understandable.
- **Static typing** that enables compile-time type checking.
- **Type inference** that reduces unnecessary type annotations.
- **Direct assembly generation** that translates Mead programs into low-level instructions.
- **Expressive language constructs** (units, `watch`, `settle`) that make common programming tasks easier to express.

The long-term goal is to deliver **Python-level simplicity with C-level performance**. Performance benchmarking against C and Python is planned.

---

## Language Features

Every statement ends with a semicolon.

### 1. Static Typing and Type Inference

Mead uses a static type system to identify incompatible operations during compilation. Variable types are inferred from their initial values, parameter types from how functions are called, and return types from `return` statements.

```mead
var age = 21;
var name = "Mead";
var active = true;
```

Supported types are integers, booleans and strings. Floating-point numbers are planned.

### 2. Variables and Assignments

```mead
var score = 100;
score = score + 25;

print(score);
```

### 3. Arithmetic and Comparisons

Mead supports arithmetic expressions and comparison operations that can be used in assignments, conditions, and loops.

```mead
var total = 10 + 20 * 3;
var passed = total >= 50;
```

### 4. Conditional Statements

`if` and `else` provide familiar, readable branching. Conditions are written in parentheses and must be boolean.

```mead
var marks = 85;

if (marks >= 90) {
    print("Excellent");
} else {
    print("Keep improving");
}
```

### 5. Loops

`while` loops execute repeatedly while a condition remains true.

```mead
var i = 0;

while (i < 5) {
    print(i);
    i = i + 1;
}
```

### 6. Functions

Functions organize reusable logic, accept up to four parameters, and return results. No types are written; they are inferred.

```mead
func square(x) {
    return x * x;
}

var result = square(5);
print(result);
```

If a function is called with two different parameter types, the compiler reports an error.

### 7. Strings and Interpolation

Any expression can be placed inside `${ }` within a string.

```mead
var name = "Mead";
print("Welcome to ${name}!");
```

---

## Signature Features

Mead includes language-level features intended to make programs safer and reduce repetitive code.

### Unit-Aware Types

Numbers can carry units using a suffix. The compiler tracks the dimension of every expression and rejects invalid operations before execution.

| Unit | Dimension | Base unit conversion |
|---|---|---|
| `m` | Length | metres |
| `km` | Length | 1000 m |
| `s` | Time | seconds |
| `min` | Time | 60 s |
| `h` | Time | 3600 s |

```mead
var distance = 2.km + 500.m;
var duration = 30.s + 2.min;
print(distance);
print(duration);
```

This prints `2500` and `150`, because all values are converted to base units at compile time.

Addition, subtraction and comparison require both sides to have the same dimension. Multiplication and division combine dimensions. Adding a distance to a duration is rejected:

```mead
var d = 5.m + 2.s;
```

```text
meadc: compilation failed: Cannot add or subtract incompatible dimensions
```

No assembly is produced when this error occurs.

### Reactive Variables with `watch`

A `watch` block runs when the watched variable changes value.

```mead
var price = 100;

watch price {
    print("Price changed: ${price}");
}

price = 120;
price = 120;
```

The block runs once. The second assignment has the same value, so the block does not run. The compiler keeps a hidden copy of the variable and compares against it after each assignment.

### The `settle` Construct

`settle` repeatedly executes a block until a complete pass produces no further changes.

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

This prints `3`. The `max` value is a maximum-iteration guard that prevents endless execution. If the block has not stabilised within the limit, the `else` block runs.

---

## Compiler Architecture

Mead uses a multi-stage compiler pipeline that translates source code into an executable program.

```text
          Mead Source Code
                  |
                  v
              Lexical Analysis
                  |
                  v
              Token Stream
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
              Object File
                  |
                  v
          Microsoft Linker
                  |
                  v
           Windows Executable
```

### Compilation Stages

1. **Lexical Analysis** — converts source text into tokens. Unit suffixes such as `.km` are part of number tokens. When `${` is found in a string, its contents are lexed and parsed as a normal expression.
2. **Parsing** — a recursive-descent parser builds the abstract syntax tree. Unit, watch and settle have their own nodes (`UnitExpr`, `WatchStmt`, `SettleStmt`) and are not rewritten into loops or conditions.
3. **Semantic Analysis** — adds type and dimension information to each expression, infers parameter and return types, and reports errors such as undefined variables, type changes on assignment and mismatched units.
4. **Code Generation** — translates the annotated AST directly into x64 assembly with no intermediate representation. It follows the Win64 calling convention (arguments in `rcx`, `rdx`, `r8`, `r9`, with 32 bytes of shadow space).
5. **Assembly** — Microsoft's `ml64.exe` converts the generated assembly into an object file.
6. **Linking** — Microsoft's linker produces the final executable.

Mead generates assembly directly rather than translating programs into C, C++, or another high-level language.

### No C Runtime

The generated program includes a minimal runtime written in assembly. Integers are printed by `mead_print_int`, output uses `WriteFile`, and the program ends with `ExitProcess`. Only `kernel32.lib` is linked, so Mead programs do not use the C runtime.

---

## Technology Stack

| Technology | Purpose |
|---|---|
| C++17 | Compiler implementation |
| CMake | Build configuration |
| x64 Assembly (MASM syntax) | Compiler output |
| Microsoft MASM (`ml64.exe`) | Assembly and object-file generation |
| Microsoft Linker (`link.exe`) | Executable generation |
| Python 3 | Test runner |
| Windows | Target execution environment |

---

## Building and Running

### Requirements

- Windows 10 or 11 (64-bit)
- Visual Studio 2022 with the "Desktop development with C++" workload (provides MSVC, `ml64.exe`, `link.exe` and the Windows SDK)
- CMake 3.16 or newer
- Python 3 (only needed for the tests)

Run all commands from the **x64 Native Tools Command Prompt for VS 2022**. This puts the assembler and linker on your path.

### Build the Compiler

```bat
git clone https://github.com/DiscoMenon/Mead.git
cd Mead

cmake -S . -B build -G "Visual Studio 17 2022" -A x64

cmake --build build --config Debug
```

This creates `build\Debug\meadc.exe`.

### Compile a Mead Program

```bat
build\Debug\meadc.exe examples\hello.mead -o build\hello.exe
```

The compiler generates `build\hello.asm`, then runs `ml64.exe` and `link.exe`, printing each step.

To stop after generating assembly, give the output an `.asm` extension:

```bat
build\Debug\meadc.exe examples\hello.mead -o build\hello.asm
```

### Run the Executable

```bat
build\hello.exe
```

---

## Testing

```bat
ctest --test-dir build -C Debug --output-on-failure
```

The test script compiles and runs 8 valid programs and checks their output. It also checks that 3 invalid programs (undefined variable, assignment that changes a variable's type, incompatible units) fail to compile with the expected error. Coverage includes arithmetic, control flow, functions, strings and interpolation, units, `watch` and `settle`.

To list each program's result, add `-V`:

```bat
ctest --test-dir build -C Debug -V
```

---

## Example

A simple Mead program:

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

Expected output:

```text
Result exceeds 25
30
```

`examples\hello.mead` uses every feature: arithmetic, conditionals, loops, functions, string interpolation, units, `watch` and `settle`.

---

## Language Quick Reference

| Feature | Syntax |
|---|---|
| Variable | `var x = 10;` |
| Assignment | `x = x + 1;` |
| Output | `print(x);` or `print("Total: ${x}");` |
| Condition | `if (x > 5) { ... } else { ... }` |
| Loop | `while (x < 10) { ... }` |
| Function | `func add(a, b) { return a + b; }` |
| Units | `2.km + 500.m` (units: `m`, `km`, `s`, `min`, `h`) |
| Watch | `watch x { ... }` |
| Settle | `settle max 10 { ... } else { ... }` |

---

## Project Structure

```text
Mead/
├── src/
│   ├── main.cpp          compiler driver (runs pipeline, ml64 and link)
│   ├── lexer.cpp/.h      tokens, unit suffixes, string interpolation
│   ├── parser.cpp/.h     recursive-descent parser
│   ├── ast.h             AST nodes
│   ├── token.h           token definitions
│   ├── semantic.cpp/.h   type inference and dimension checks
│   └── codegen.cpp/.h    x64 assembly generation
├── examples/
│   └── hello.mead
├── tests/
│   ├── run_tests.py
│   └── *.mead            valid and invalid test programs
├── CMakeLists.txt
└── README.md
```

---

## Planned Features

The following are not implemented yet.

### Arrays

Indexed collections of values, with dynamic sizing, and `for` loops to iterate over them.

### Error Handling

Mead plans to distinguish recoverable failures from serious programming errors.

| Construct | Purpose |
|---|---|
| `Ok(value)` / `Err(error)` | Result values for success or failure |
| `match` | Handle success and failure explicitly |
| `?` | Propagate an error to the calling function |
| `??` | Provide a fallback value when an operation fails |

### Other Planned Work

| Area | Planned work |
|---|---|
| Types | Floating-point arithmetic and more numeric types |
| `watch` | Dependent watching, so watchers on derived variables are called |
| Operators | Short-circuit evaluation for `&&` and `||` |
| Output | Printing of full 64-bit integers |
| Optimisation | Constant folding, register allocation, removing redundant moves |
| Diagnostics | Line and column numbers in error messages |
| Evaluation | Benchmarks against equivalent C and Python programs |

---

## Why Mead?

Mead is an exploration of what a programming language can offer when its syntax, type system, and compilation strategy are designed together.

By combining readable source code, compile-time checking, distinctive language constructs, and direct assembly generation, Mead aims to make the gap between writing a program and executing it smaller.

**Mead — Python-level simplicity with C-level performance.**