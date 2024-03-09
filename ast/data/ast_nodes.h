#pragma once
#include "node_defs.h"
#include "abstract_data.h"

#include <vector>
#include <memory>

namespace ast::nodes {
    struct node {};

    struct root : node {
        std::vector<function> functions;
    };

    struct expression : node {
        std::unique_ptr<op> operation;
    };

    struct code_block : node {
        std::vector<expression> expressions;
    };

    struct function : node {
        value_type return_type;
        std::vector<value_type> param_types;

        code_block body;
    };

    struct conditional {
        expression condition;
        code_block body;
    };

    struct if_cond : conditional {};

    struct while_cond : conditional {};

    struct initialization : expression {
        bool is_const;
        value_type type;
        std::string_view variable_name;
    };

    struct method_call : expression {
        std::string_view method_name;
        std::vector<expression> arguments;
    };

    struct op : node {};

    struct return_op : op {
        expression value;
    };

    struct assignment : op {
        std::variant<initialization, std::string_view> variable;
        expression value;
    };

    struct un_op : op {
        std::string_view operation;
        std::unique_ptr<expression> operand;
    };

    struct bin_op : op {
        std::string_view operation;
        std::unique_ptr<expression> left;
        std::unique_ptr<expression> right;
    };

    struct literal : expression {};

    struct string_literal : literal {
        std::string_view value;
    };

    struct int_literal : literal {
        int64_t value;
    };

    struct uint_literal : literal {
        uint64_t value;
    };

    struct fp_literal : literal {
        double_t value;
    };

    struct char_literal : literal {
        char value;
    };
}
