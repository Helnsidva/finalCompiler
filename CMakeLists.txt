cmake_minimum_required(VERSION 3.9)
project(finalCompiler C)

set(CMAKE_C_STANDARD 99)

add_executable(finalCompiler main.c scanner.c parser.c codeGenerator.c scanner.h parser.h codeGenerator.h)