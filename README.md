# Mead

### Python-level simplicity. C-level performance.

Mead is a programming language built from scratch in C++, designed to combine the simplicity and readability of high-level languages with the execution performance of compiled languages.

Mead features a clean syntax, static typing with type inference, expressive control flow, and a compiler that generates x86-64 assembly directly. The generated assembly is processed using Microsoft's assembler and linker to produce standalone Windows executables.

The goal is simple: **make programming easier to write without sacrificing the benefits of compilation.**

---

## Table of Contents

- [Vision](#vision)
- [Language Features](#language-features)
- [Signature Features](#signature-features)
- [Error Handling](#error-handling)
- [Compiler Architecture](#compiler-architecture)
- [Technology Stack](#technology-stack)
- [Building and Running](#building-and-running)
- [Example](#example)
- [Project Structure](#project-structure)

---

## Vision

Programming languages often involve a trade-off between development simplicity and execution performance.

Python offers concise syntax, dynamic flexibility, and a low barrier to entry, but its standard implementation incurs runtime overhead. C and C++ provide efficient compiled execution but often require more explicit type declarations and memory management.

Mead explores a middle ground through:

- **Readable syntax** that keeps programs concise and understandable.
- **Static typing** that enables compile-time type checking.
- **Type inference** that reduces unnecessary type annotations.
- **Direct assembly generation** that translates Mead programs into low-level instructions.
- **Expressive language constructs** that make common programming tasks easier to express.

The long-term vision is to deliver **Python-level simplicity with C-level performance**.

## Language Features

### 1. Static Typing and Type Inference

Mead uses a static type system to identify incompatible operations during compilation. Type inference reduces the need to explicitly annotate every variable.

```mead
var age = 21
var price = 99.5
var name = "Mead"
var active = true
```

Types are inferred from initial values, combining concise declarations with compile-time type checking.

### 2. Variables and Assignments

Variables provide a straightforward way to store and update values.

```mead
var score = 100
score = score + 25

print(score)
```

### 3. Arithmetic and Comparisons

Mead supports arithmetic expressions and comparison operations that can be used in assignments, conditions, and loops.

```mead
var total = 10 + 20 * 3
var passed = total >= 50
```

### 4. Conditional Statements

`if` and `else` provide familiar, readable branching.

```mead
var marks = 85

if marks >= 90 {
    print("Excellent")
} else {
    print("Keep improving")
}
```

### 5. Loops

`while` loops allow code to execute repeatedly while a condition remains true.

```mead
var i = 0

while i < 5 {
    print(i)
    i = i + 1
}
```

### 6. Functions

Functions organize reusable logic, accept parameters, and return results.

```mead
fn square(x: int) {
    return x * x
}

var result = square(5)
print(result)
```

Return types can be inferred from return expressions when omitted.

### 7. Arrays

Arrays provide indexed access to collections of values, with a design that supports dynamic sizing.

```mead
var numbers = [10, 20, 30]
```

### 8. Strings and Interpolation

Strings provide readable text handling, with interpolation designed to make combining text and expressions convenient.

```mead
var name = "Mead"
print("Welcome to {name}!")
```

---

## Signature Features

Mead explores language-level features intended to make programs safer and reduce repetitive code.

### Unit-Aware Types

Unit-aware types associate values with physical dimensions such as distance, time, and mass. The compiler can use these dimensions to detect invalid operations before execution.

```text
distance = 10 meters
time = 2 seconds

speed = distance / time
```

Operations involving incompatible dimensions, such as adding a distance to a duration, are rejected by the type system. Compatible unit conversions can be handled explicitly or by the language's unit rules.

### Reactive Variables

Reactive variables allow derived values to respond automatically when their dependencies change.

```text
var price = 100
var quantity = 3

watch total = price * quantity
```

When a dependency changes, the derived value is recalculated. Watchers respond to actual value changes rather than running unnecessarily when the value remains unchanged.

This model is useful for dependency-driven calculations and state that must remain synchronized.

### The `settle` Construct

The `settle` construct repeatedly executes a block until a complete pass produces no further changes.

```text
settle {
    update_dependencies()
    recalculate_values()
}
```

A maximum-iteration guard prevents endless execution. If the system fails to stabilize within the configured limit, the construct can report non-convergence and execute its designated failure path.

This provides a structured approach to iterative calculations and dependency resolution.

---

## Error Handling

Mead distinguishes recoverable failures from serious programming errors.

### Result Values

Operations that can fail return a result representing either success or failure:

- `Ok(value)` — the operation succeeded.
- `Err(error)` — the operation failed.

This allows programs to handle expected failures without relying on uncontrolled termination.

### Pattern Matching with `match`

The `match` construct handles success and failure explicitly.

```text
match result {
    Ok(value) => print(value)
    Err(error) => print(error)
}
```

### Error Propagation with `?`

The `?` operator propagates an error to the calling function, avoiding repetitive error-checking code.

### Fallback Values with `??`

The `??` operator provides a fallback when an operation produces an error or an unavailable result, according to the language's result-handling rules.

Recoverable failures, such as file-access errors, can be handled as part of normal program logic. Serious failures, such as stack overflow or violated internal invariants, are treated separately.

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
                  |
                  v
            Code Generation
                  |
                  v
           x86-64 Assembly
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

1. **Lexical Analysis** — converts source text into tokens such as identifiers, keywords, literals, and operators.
2. **Parsing** — analyzes token sequences according to the language grammar and builds an abstract syntax tree.
3. **Semantic Analysis** — checks the meaning of the program, including variable usage and type compatibility.
4. **Code Generation** — translates the program representation into x86-64 assembly instructions.
5. **Assembly** — Microsoft's `ml64.exe` assembler converts the generated assembly into an object file.
6. **Linking** — Microsoft's linker resolves external references and produces the final executable.

Mead generates assembly directly rather than translating programs into C, C++, or another high-level language.

---

## Technology Stack

| Technology | Purpose |
|---|---|
| C++ | Compiler implementation |
| CMake | Build configuration |
| x86-64 Assembly | Compiler output |
| Microsoft MASM (`ml64.exe`) | Assembly and object-file generation |
| Microsoft Linker | Executable generation |
| Windows | Target execution environment |

---

## Building and Running

### Requirements

- Windows
- Visual Studio 2022 Build Tools with the C++ toolchain
- CMake
- Microsoft MASM (`ml64.exe`) and the Microsoft linker

### Build the Compiler

Open the **x64 Native Tools Command Prompt for Visual Studio 2022** and run:

```bat
cd /d "%USERPROFILE%\Desktop\Mead"

cmake -S . -B build -G "Visual Studio 17 2022" -A x64

cmake --build build --config Debug
```

### Compile a Mead Program

```bat
build\Debug\meadc.exe examples\hello.mead -o build\hello.exe
```

### Run the Executable

```bat
build\hello.exe
```

The compiler generates assembly from the Mead source, after which the assembler and linker produce the executable.

---

## Example

A simple Mead program:

```mead
var x = 10
var y = 20
var result = x + y

if result > 25 {
    print("Result exceeds 25")
} else {
    print("Result is 25 or less")
}

print(result)
```

Expected output:

```text
Result exceeds 25
30
```

---

## Project Structure

```text
Mead/
├── src/
│   ├── main.cpp
│   ├── lexer.cpp
│   └── ...
├── include/
├── examples/
│   └── hello.mead
├── CMakeLists.txt
└── README.md
```

The source tree separates compiler implementation, language examples, and build configuration. Additional modules implement parsing, semantic analysis, and code generation as the compiler architecture develops.

---

## Why Mead?

Mead is an exploration of what a programming language can offer when its syntax, type system, and compilation strategy are designed together.

By combining readable source code, compile-time checking, distinctive language constructs, and direct assembly generation, Mead aims to make the gap between writing a program and executing it smaller.

**Mead — Python-level simplicity with C-level performance.**
