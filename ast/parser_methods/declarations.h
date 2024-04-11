#pragma once

#include "../data/ast_nodes.h"
#include "../util.h"

namespace ast::pm {
    nodes::struct_declaration parse_struct_decl(lex_cptr &ptr, const lex_cptr end);
    std::vector<nodes::type_instance> parse_struct_types(lex_cptr &ptr, const lex_cptr end);
}
