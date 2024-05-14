#pragma once

#include "../data/ast_nodes.h"
#include "../util.h"

namespace ast::pm {
    std::unique_ptr<nodes::statement> parse_statement(lex_cptr &ptr, lex_cptr end);

    nodes::loop parse_loop(lex_cptr &ptr, lex_cptr end);

    nodes::for_loop parse_for_loop(lex_cptr &ptr, lex_cptr end);

    nodes::if_statement parse_if_statement(lex_cptr &ptr, lex_cptr end);
}
