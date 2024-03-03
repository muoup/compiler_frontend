#include "interface.h"

#include <format>
#include "../lexer/lex.h"
#include "parser_methods/program.h"

using namespace ast;

ast_node ast::parse(const std::vector<lex::lex_token> &tokens) {
    ast_node root { ast_node_type::ROOT };

    auto ptr = tokens.cbegin();
    const auto end = tokens.cend();

    while (ptr < end) {
        root.add_child(
            pm::parse_method(ptr, end)
        );
    }

    return root;
}