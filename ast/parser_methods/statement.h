#pragma once

#include "../declarations.h"

namespace ast::pm {
    ast_node parse_statement(lex_cptr& ptr, lex_cptr end);

    ast_node parse_conditional(lex_cptr& ptr, lex_cptr end);

    ast_node parse_initialization(lex_cptr& ptr, lex_cptr end);
    ast_node parse_expression(lex_cptr& ptr, lex_cptr end);

    ast_node parse_value(lex_cptr &ptr);

    void try_optimize(ast_node& node);
}
