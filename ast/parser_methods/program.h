#pragma once
#include "../declarations.h"

namespace ast::pm {
    ast_node parse_method(lex_cptr& ptr, lex_cptr end);
    ast_node parse_body(lex_cptr& ptr, lex_cptr end);

    ast_node parse_method_params(lex_cptr& ptr, lex_cptr end);
    ast_node parse_call_params(lex_cptr& ptr, lex_cptr end);
}
