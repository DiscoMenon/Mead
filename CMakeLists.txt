
cmake_minimum_required(VERSION 3.16)
project(Mead LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(meadc
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/semantic.cpp
    src/codegen.cpp
)

include(CTest)

if(BUILD_TESTING)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    add_test(
        NAME mead_integration
        COMMAND
            "${Python3_EXECUTABLE}"
            "${CMAKE_SOURCE_DIR}/tests/run_tests.py"
            --compiler "$<TARGET_FILE:meadc>"
            --source-dir "${CMAKE_SOURCE_DIR}/tests"
            --work-dir "${CMAKE_BINARY_DIR}/test-work"
    )
endif()
