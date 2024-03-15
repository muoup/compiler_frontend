#pragma once
#include <memory>

#include "node_defs.h"
#include "abstract_data.h"

#include <vector>
#include <optional>

namespace ast::nodes {
    struct root {
        std::vector<function> functions;
        std::vector<initialization> global_vars;

        // TODO: Struct and enum definitions

        void print(size_t depth) const;
    };

    struct code_block {
        std::vector<statement> statements;

        void print(size_t depth) const;
    };

    struct function {
        value_type return_type;
        std::string_view function_name;
        std::vector<initialization> param_types;

        code_block body;

        void print(size_t depth) const;
    };

    struct variable {
        std::string_view name;

        void print(size_t depth) const;
    };

    struct initialization {
        value_type type;
        variable var;

        void print(size_t depth) const;
    };

    struct method_call {
        std::string_view method_name;
        std::vector<expression> arguments;

        void print(size_t depth) const;
    };

    struct bin_op {
        bin_op_type type;
        bool assignment;
        std::unique_ptr<expression> left;
        std::unique_ptr<expression> right;

        void print(size_t depth) const;
    };

    struct un_op {
        un_op_type type;
        std::unique_ptr<expression> value;

        void print(size_t depth) const;
    };

    enum literal_type {
        UINT, INT,
        DOUBLE,
        CHAR,
        STRING
    };
    struct literal {
        std::variant<unsigned int, int, double, char, std::string_view> value;

        void print(size_t depth) const;
    };

    enum assignment_type {
        ASSIGN_INITIALIZATION,
        ASSIGN_VARIABLE
    };
    struct assignment {
        std::variant<initialization, variable> variable;
        std::unique_ptr<expression> value;
        bin_op_type operation;

        void print(size_t depth) const;
    };

    enum expression_type {
        METHOD_CALL,
        ASSIGNMENT, VARIABLE, INITIALIZATION,
        UN_OP, BIN_OP,
        LITERAL
    };
    struct expression {
        std::variant<method_call, assignment, variable,
                    initialization, un_op, bin_op, literal> value;

        void print(size_t depth) const;
    };

    enum class conditional_type {
        IF_STATEMENT,
        WHILE_LOOP,
        DO_WHILE_LOOP
    };
    struct conditional {
        conditional_type type;
        expression condition;
        code_block body;

        void print(size_t depth) const;
    };

    struct return_op {
        std::optional<expression> value;

        void print(size_t depth) const;
    };

    enum statement_type {
        RETURN,
        EXPRESSION,
        CONDITIONAL,
    };
    struct statement {
        std::variant<return_op, expression, conditional> value;

        void print(size_t depth) const;
    };
}
