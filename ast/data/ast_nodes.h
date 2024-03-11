#pragma once
#include "node_defs.h"
#include "abstract_data.h"

#include <vector>
#include <memory>
#include <optional>

namespace ast::nodes {
    struct root {
        std::vector<function> functions;
        std::vector<initialization> global_vars;

        // TODO: Struct and enum definitions
    };

    struct code_block {
        std::vector<statement> expressions;
    };

    struct function {
        value_type return_type;
        std::string_view function_name;
        std::vector<value_type> param_types;

        code_block body;
    };

    struct variable {
        std::string_view name;
    };

    struct initialization {
        value_type type;
        variable var;
    };

    struct method_call {
        std::string_view method_name;
        std::vector<expression> arguments;
    };

    struct op {
        operator_type type;
        std::vector<expression> operands;
    };

    struct literal {
        std::variant<unsigned int, int, double, char, std::string_view> value;
    };

    struct assignment {
        std::variant<initialization, std::string_view> variable;
        std::string_view value;
    };

    struct expression {
        std::variant<method_call, assignment, variable, initialization, op, literal> value;
    };

    enum conditional_type {
        IF_STATEMENT,
        WHILE_LOOP,
        DO_WHILE_LOOP
    };
    struct conditional {
        conditional_type type;
        expression condition;
        code_block body;
    };

    struct return_op {
        std::optional<expression> value;
    };

    struct statement {
        std::variant<return_op, expression, conditional, initialization> value;
    };
}
