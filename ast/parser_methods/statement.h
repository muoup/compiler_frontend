#pragma once

#include "../data/ast_nodes.h"
#include "../declarations.h"

namespace ast::pm {
    nodes::statement parse_statement(lex_cptr &ptr, lex_cptr end);

    nodes::conditional parse_conditional(lex_cptr &ptr, lex_cptr end);

    nodes::initialization parse_initialization(lex_cptr &ptr, lex_cptr end);

    nodes::un_op parse_unop(lex_cptr &ptr, lex_cptr end);

    nodes::bin_op parse_binop(lex_cptr &ptr, lex_cptr end);

    nodes::expression parse_expression(lex_cptr &ptr, lex_cptr end);

    nodes::method_call parse_method_call(lex_cptr &ptr, lex_cptr end);

    std::optional<nodes::expression> parse_value(lex_cptr &ptr, lex_cptr end);

    std::optional<nodes::literal> parse_literal(lex_cptr &ptr, lex_cptr end);

    void try_optimize(ast_node &node);
}
