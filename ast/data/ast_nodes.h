#pragma once
#include <memory>

#include "node_defs.h"
#include "abstract_data.h"

#include <vector>
#include <optional>

namespace ast::nodes {
    struct root {
        std::vector<function> functions;
        std::vector<type_instance> global_vars;

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
        std::vector<type_instance> param_types;

        code_block body;

        void print(size_t depth) const;
    };

    struct type_instance {
        value_type type;
        std::string_view var_name;

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

    struct var_modification {
        bool dereferenced;
        std::string_view var_name;
        std::unique_ptr<expression> value;
        std::optional<bin_op_type> additional_op;

        void print(size_t depth) const;
    };

    struct initialization {
        type_instance variable;
        std::optional<std::unique_ptr<expression>> value;

        void print(size_t depth) const;
    };

    struct var_ref {
        std::string_view name;

        void print(size_t depth) const;
    };

    enum expression_type {
        METHOD_CALL,
        INITIALIZATION, VAR_MODIFICATION,
        VAR_REF, TYPE_INSTANCE,
        UN_OP, BIN_OP,
        LITERAL
    };
    struct expression {
        std::variant<method_call, initialization, var_modification, var_ref,
                    type_instance, un_op, bin_op, literal> value;

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
