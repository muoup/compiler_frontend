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
     *  Code Block: Non-Abstract Data Type
     *  ----------------------------------
     *  A code block represents a scope, any code blocks
     *  within will have their own innermost scope but will
     *  also be able to reference any outer scopes.
     */
    struct scope_block : codegen_node {
        using scope_stmts = std::vector<std::unique_ptr<statement>>;
        scope_stmts statements;

        scope_block() = default;
        scope_block(scope_block&&) noexcept = default;
        scope_block(scope_stmts statements)
            : statements(std::move(statements)) {}

        ~scope_block() = default;
        void print(size_t depth) const override;
        llvm::BasicBlock* generate_code(cg::scope_data &scope) const override;
    };

    /**
     *  Function: Non-Abstract Data Type
     *  -------------------------------
     *  A function represents a callable block of code at the top level of the program.
     *  Functionally this acts very similar to a statement, but differs in how it must
     *  be implemented into a program. In the future, if functions are able to be implemented
     *  within a function, the line between a function and a statement will blur. But for now,
     *  functions are a top-level construct.
     */
    struct function : codegen_node {
        value_type return_type;
        std::string_view fn_name;
        std::vector<type_instance> param_types;
        scope_block body;

        function(function&&) noexcept = default;
        function(value_type return_type, std::string_view method_name, std::vector<type_instance> param_types, scope_block body)
            : return_type(return_type), fn_name(method_name), param_types(std::move(param_types)), body(std::move(body)) {}

        ~function() = default;
        void print(size_t depth) const override;
        CODEGEN() override;
    };
}
