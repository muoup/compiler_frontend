#include "interface.h"

#include <format>
#include "../lexer/lex.h"
#include "data/ast_nodes.h"
#include "parser_methods/program.h"

using namespace ast;

nodes::root ast::parse(const std::vector<lex::lex_token> &tokens) {
    nodes::root root = {};

    auto ptr = tokens.cbegin();
    const auto end = tokens.cend();

    while (ptr < end) {
        root.functions
            .emplace_back(pm::parse_method(ptr, end));
    }

    return root;
}