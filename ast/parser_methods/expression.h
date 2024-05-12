#pragma once

#include <memory>
#include "../util.h"
#include "../data/node_interfaces.h"
#include "../data/ast_nodes.h"

namespace ast::nodes {
    struct literal;
    struct un_op;
    struct bin_op;
    struct method_call;
    struct var_ref;
    struct match;
}

namespace ast::pm {
    nodes::type_instance parse_type_instance(lex_cptr &ptr, lex_cptr end);

    std::unique_ptr<nodes::expression> parse_unop(lex_cptr &ptr, const lex_cptr end);

    std::optional<nodes::bin_op_type> parse_binop(lex_cptr &ptr, const lex_cptr);

    std::optional<nodes::bin_op_type> parse_assn(const ast::lex_cptr, ast::lex_cptr &ptr);

    nodes::method_call parse_method_call(lex_cptr &ptr, lex_cptr end);

    nodes::var_ref parse_variable(lex_cptr &ptr, lex_cptr end);

    nodes::bin_op parse_array_access(lex_cptr &ptr, const lex_cptr end);

    std::optional<nodes::literal> parse_literal(lex_cptr &ptr, lex_cptr end);

    nodes::match parse_match(lex_cptr &ptr, lex_cptr end);

    nodes::initializer_list parse_initializer_list(lex_cptr &ptr, const ast::lex_cptr end);

    nodes::struct_initializer parse_struct_initializer(ast::lex_cptr &ptr, const ast::lex_cptr end);

    std::unique_ptr<nodes::expression> parse_expression(lex_cptr &ptr, const lex_cptr end);

    std::unique_ptr<nodes::expression> parse_expr_tree(lex_cptr &ptr, const lex_cptr end);
}
