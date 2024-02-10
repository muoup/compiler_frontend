cmake_minimum_required(VERSION 3.27)
project(LLVMProj)

set(CMAKE_CXX_STANDARD 23)

add_executable(LLVMProj main.cpp
        lexer/lex.cpp
        lexer/lex.h
        lexer/literal_cache.cpp
        lexer/literal_cache.h)
