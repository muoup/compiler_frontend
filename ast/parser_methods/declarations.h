#pragma once

#include "../data/ast_nodes.h"
#include "../util.h"

namespace ast::pm {
    std::unique_ptr<nodes::struct_declaration> parse_struct_decl(lex_cptr &ptr, const lex_cptr end);
    std::vector<nodes::type_instance> parse_split_type_inst(lex_cptr &ptr, const lex_cptr end);
}
