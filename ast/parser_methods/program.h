#pragma once

#include "../util.h"
#include "../data/node_interfaces.h"
#include "../data/abstract_data.h"
#include "../data/ast_nodes.h"

namespace ast::pm {
    std::unique_ptr<nodes::program_level_stmt> parse_program_level_stmt(lex_cptr& ptr, lex_cptr end);

    nodes::function parse_function(lex_cptr& ptr, const lex_cptr end);
    nodes::scope_block parse_body(lex_cptr& ptr, lex_cptr end);
    nodes::variable_type parse_var_type(lex_cptr& ptr, lex_cptr end);
    nodes::struct_declaration parse_struct_decl(lex_cptr& ptr, lex_cptr end);

    std::vector<nodes::type_instance> parse_method_params(lex_cptr& ptr, lex_cptr end);
    std::vector<std::unique_ptr<nodes::expression>> parse_call_params(lex_cptr& ptr, const lex_cptr end);
}
