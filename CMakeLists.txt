cmake_minimum_required(VERSION 3.29)
project(pl0c)

set(CMAKE_CXX_STANDARD 17)

add_executable(pl0c main.cpp
        lexer.h
        token.h
        ASTNode.h
        Parser.h
        SymbolTable.h
        ByteCode.h
        Interpreter.h
)
