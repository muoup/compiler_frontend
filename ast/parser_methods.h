#pragma once
#include "analyze_util.h"

namespace ast {
    struct ast_node;

    using parse_fn = ast_node(*)(lex_cptr&, lex_cptr);

    ast_node parse_conditional(lex_cptr& ptr, lex_cptr end);
    ast_node parse_assignment(lex_cptr& ptr, lex_cptr end);
    ast_node parse_body(lex_cptr& ptr, lex_cptr end);
    ast_node parse_method_params(lex_cptr& ptr, lex_cptr end);
    ast_node parse_method(lex_cptr& ptr, lex_cptr end);

    ast_node parse_between(lex_cptr& ptr, parse_fn fn);
}
