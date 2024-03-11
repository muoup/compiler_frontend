#pragma once
#include "../declarations.h"
#include "../data/ast_nodes.h"
#include "../data/abstract_data.h"

namespace ast::pm {
    nodes::function parse_method(lex_cptr& ptr, lex_cptr end);
    nodes::code_block parse_body(lex_cptr& ptr, lex_cptr end);

    nodes::value_type parse_value_type(lex_cptr& ptr, lex_cptr end);

    std::vector<nodes::value_type> parse_method_params(lex_cptr& ptr, lex_cptr end);
    std::vector<nodes::expression> parse_call_params(lex_cptr& ptr, lex_cptr end);
}
