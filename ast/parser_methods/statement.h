#pragma once

#include "../data/ast_nodes.h"
#include "../data/abstract_data.h"
#include "../declarations.h"

namespace ast::pm {
    nodes::expression parse_statement(lex_cptr& ptr, lex_cptr end);

    nodes::conditional parse_conditional(lex_cptr& ptr, lex_cptr end);

    nodes::initialization parse_initialization(lex_cptr& ptr, lex_cptr end);
    nodes::op parse_expression(lex_cptr& ptr, lex_cptr end);

    nodes::expression parse_value(lex_cptr &ptr, lex_cptr end);

    void try_optimize(ast_node& node);
}
