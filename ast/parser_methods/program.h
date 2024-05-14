#pragma once

#include "../util.h"
#include "../data/node_interfaces.h"
#include "../data/abstract_data.h"
#include "../data/ast_nodes.h"

namespace ast::pm {
    std::unique_ptr<nodes::program_level_stmt> parse_program_level_stmt(lex_cptr& ptr, lex_cptr end);

    std::unique_ptr<nodes::function_prototype> parse_function_prototype(lex_cptr &ptr, lex_cptr end);
    std::unique_ptr<nodes::function> parse_function(lex_cptr& ptr, const lex_cptr end);
    nodes::scope_block parse_body(lex_cptr& ptr, lex_cptr end);
    nodes::variable_type parse_var_type(lex_cptr& ptr, lex_cptr end);
    std::unique_ptr<nodes::struct_declaration> parse_struct_decl(lex_cptr& ptr, const lex_cptr end);

    ast::nodes::method_params parse_method_params(lex_cptr& ptr, const lex_cptr end);
    std::vector<std::unique_ptr<nodes::expression>> parse_expression_list(lex_cptr& ptr, const lex_cptr end);
}
