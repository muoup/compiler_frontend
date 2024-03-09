#pragma once

namespace ast::nodes {
    struct node;

    struct root;

    struct invokable;
    struct function;
    struct if_cond;
    struct while_cond;

    struct code_block;

    struct expression;
    struct assignment;
    struct initialization;
    struct method_call;

    struct op;
    struct un_op;
    struct bin_op;
    struct return_op;

    struct literal;
    struct string_literal;
    struct int_literal;
    struct fp_literal;
    struct char_literal;
};