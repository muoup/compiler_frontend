#pragma once

#include "../data/ast_nodes.h"
#include "../util.h"

namespace ast::pm {
    std::unique_ptr<nodes::statement> parse_statement(lex_cptr &ptr, lex_cptr end);

    nodes::loop parse_loop(lex_cptr &ptr, lex_cptr end);

    nodes::for_loop parse_for_loop(lex_cptr &ptr, lex_cptr end);

    nodes::if_statement parse_if_statement(lex_cptr &ptr, lex_cptr end);

    nodes::type_instance parse_type_instance(lex_cptr &ptr, lex_cptr end);

    nodes::un_op parse_unop(lex_cptr &ptr, lex_cptr end);

    nodes::bin_op parse_binop(lex_cptr &ptr, lex_cptr end);

    nodes::method_call parse_method_call(lex_cptr &ptr, lex_cptr end);

    std::unique_ptr<nodes::expression> parse_value(lex_cptr &ptr, const lex_cptr end);

    std::optional<nodes::literal> parse_literal(lex_cptr &ptr, lex_cptr end);
}
