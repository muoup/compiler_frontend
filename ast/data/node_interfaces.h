#pragma once
#include <memory>

#include "abstract_data.h"

#include <vector>
#include <optional>
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>

#define CODEGEN() llvm::Value* generate_code(cg::scope_data &scope) const

namespace cg {
    struct scope_data;
}

namespace ast::nodes {

    /**
     *  Codegen Node: Abstract Data Interface
     *  -------------------------------------
     *  A codegen node is any node which can be
     *  placed into the abstract syntax tree, and
     *  has with it a code generation method when it
     *  is to be compiled into LLVM IR.
     */
    struct codegen_node {
        virtual ~codegen_node() = default;
        virtual void print(size_t depth) const = 0;
        virtual llvm::Value* generate_code(cg::scope_data &scope) const = 0;
    };

    /**
     *  Expression: Abstract Data Type
     *  ------------------------------
     *  An expression is a single segment of a statement,
     *  if this is generated via codegen, it will return a
     *  referencable value of some kind.
     */
    struct expression : codegen_node {
        expression() = default;

        ~expression() override = default;
        void print(size_t depth) const override = 0;
        CODEGEN() override = 0;

        virtual value_type get_type() const = 0;
    };

    /**
     *  Statement: Abstract Data Type
     *  -----------------------------
     *  A statement represents some executable part of the code,
     *  usually this is one single line of code, but may additionally
     *  connect to a block of code (i.e. a conditional statement).
     */
    struct statement : codegen_node {
        statement() = default;
        statement(statement&&) noexcept = default;

        virtual ~statement() = default;
        virtual void print(size_t depth) const override = 0;
        CODEGEN() override = 0;
    };

    /**
     *  Program Level Statement: Abstract Data Type
     *  -------------------------------------------
     *  A program-level statement is something which can be executed from the
     *  root of the program. This is usually either a prototype (struct or function),
     *  a function definition, or a global variable declaration.
     */
    struct program_level_stmt : codegen_node {
        program_level_stmt() = default;
        program_level_stmt(program_level_stmt&&) noexcept = default;

        virtual ~program_level_stmt() = default;
        virtual void print(size_t depth) const override = 0;
        CODEGEN() override = 0;
    };
}
