#pragma once
#include <memory>

#include <span>
#include <vector>
#include <optional>

#ifdef LLVM_ENABLE

#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <iostream>

#define CG_BASE() generate_code(cg::scope_data &scope) const

#define CG_PURE() virtual llvm::Value* CG_BASE() = 0

#define CG_CUSTOMGEN(return_type) return_type CG_BASE() override
#define CG_BASICGEN() CG_CUSTOMGEN(llvm::Value*)

#else

#define CG_BASE()
#define CG_PURE()
#define CG_CUSTOMGEN(return_type)
#define CG_BASICGEN()

#endif

#define NODENAME(str) std::string_view node_name() const override { return str; }
#define ABSTRACT_NODENAME(str) std::string_view node_name() const override { return str "(SHOULD NOT PRINT)"; }
#define CHILDREN(...) \
       cg_container children() const override { \
            return cg_container() \
                .add(__VA_ARGS__); \
        }
#define DETAILS(...) \
    bool has_print_details() const override { return true; };                 \
    void print_details() const override { \
        __va_print(__VA_ARGS__);                \
    }

namespace cg {
    struct scope_data;
}

namespace ast::nodes {
    template<class, template<class...> class>
    inline constexpr bool is_specialization = false;
    template<template<class...> class T, class... Args>
    inline constexpr bool is_specialization<T<Args...>, T> = true;

    struct printable;
    struct variable_type;

    struct cg_container {
        std::vector<const printable*> nodes {};

        cg_container() = default;

        template<typename T>
        cg_container&& add(const std::vector<T> &list) {
            for (const auto &n : list) {
                add(n);
            }

            return std::move(*this);
        }

        template<typename T>
        cg_container&& add(const std::unique_ptr<T> &node) {
            if (node)
                add(*node);
            return std::move(*this);
        }

        template<typename T>
        cg_container&& add(const std::optional<T> &node) {
            if (node)
                add(*node);
            return std::move(*this);
        }

        template<typename T>
        cg_container&& add(const std::span<T> &list) {
            for (const auto &n : list) {
                add(n);
            }
            return std::move(*this);
        }

        template<typename T, std::enable_if<std::is_base_of_v<printable, T>>>
        cg_container&& add(const T &node) {
            nodes.push_back(&node);

            return std::move(*this);
        }

        template<typename... Nodes>
        cg_container&& add(const Nodes &...node) {
            (add(node), ...);
            return std::move(*this);
        }
    };

    struct printable {
        virtual std::string_view node_name() const {
            throw std::runtime_error("Printable node does not have a name");
        }

        virtual bool has_print_details() const { return false; };
        virtual void print_details() const { };

        virtual cg_container children() const {
            return cg_container();
        }

        void print(size_t depth = 0) const;
        virtual ~printable() = default;

        template <typename ...T>
        static void __va_print(const T &... var_args) {
            (__print(var_args), ...);
        }

        template <typename T>
        static void __print(const T& content) {
            if constexpr (ast::nodes::is_specialization<T, std::unique_ptr> || ast::nodes::is_specialization<T, std::optional>) {
                if (content) {
                    __print(*content);
                }
            } else if constexpr (std::is_base_of_v<printable, T>) {
                content.print_details();
            } else {
                std::cout << content << " ";
            }
        }
    };

    /**
     *  Codegen Node: Abstract Node Interface
     *  -------------------------------------
     *  A codegen node is any node which can be
     *  placed into the abstract syntax tree, and
     *  has with it a code generation method when it
     *  is to be compiled into LLVM IR.
     */
    struct codegen_node : printable {
        ABSTRACT_NODENAME("CODEGEN_NODE (SHOULD NOT BE PRINTED)")

        CG_PURE();
    };

    /**
     *  Expression: Abstract Node Interface
     *  ------------------------------
     *  An expression is a single segment of a statement,
     *  if this is generated via codegen, it will return a
     *  referencable value of some kind.
     */
    struct expression : codegen_node {
        ABSTRACT_NODENAME("EXPRESSION");

        expression() = default;
        ~expression() override = default;

        virtual variable_type get_type() const = 0;
    };

    /**
     *  Statement: Abstract Node Interface
     *  -----------------------------
     *  A statement represents some executable part of the code,
     *  usually this is one single line of code, but may additionally
     *  connect to a block of code (i.e. a conditional statement).
     */
    struct statement : codegen_node {
        ABSTRACT_NODENAME("STATEMENT (SHOULD NOT BE PRINTED)");

        statement() = default;
        statement(statement&&) noexcept = default;

        virtual ~statement() = default;
    };

    /**
     *  Program Level Statement: Abstract Node Interface
     *  -------------------------------------------
     *  A program-level statement is something which can be executed from the
     *  root of the program. This is usually either a prototype (struct or function),
     *  a function definition, or a global variable declaration.
     */
    struct program_level_stmt : codegen_node {
        ABSTRACT_NODENAME("PROGRAM_LEVEL_STMT (SHOULD NOT BE PRINTED)");

        program_level_stmt() = default;
        program_level_stmt(program_level_stmt&&) noexcept = default;

        virtual ~program_level_stmt() = default;
    };

    /**
     *  Code Block: Non-Abstract Node Interface
     *  ----------------------------------
     *  A code block represents a scope, any code blocks
     *  within will have their own innermost scope but will
     *  also be able to reference any outer scopes. Whether or
     *  not a scope block can be realized in a more specific
     *  form I am not sure.
     */
    struct scope_block : codegen_node {
        NODENAME("SCOPE_BLOCK");
        CHILDREN(statements);

        using scope_stmts = std::vector<std::unique_ptr<statement>>;
        scope_stmts statements;

        scope_block() = default;
        scope_block(scope_block&&) noexcept = default;
        scope_block(scope_stmts statements)
                : statements(std::move(statements)) {}

        ~scope_block() = default;
        CG_CUSTOMGEN(llvm::BasicBlock*);
    };
}
