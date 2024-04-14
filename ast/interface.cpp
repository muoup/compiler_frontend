#include <iostream>

#include "interface.h"
#include "parser_methods/program.h"

using namespace ast;

nodes::root ast::parse(const std::vector<lex::lex_token> &tokens) {
    nodes::root root {};

    scope_stack.clear();
    scope_stack.emplace_back();

    struct_types.clear();

    function_types.clear();

    auto ptr = tokens.cbegin();
    const auto end = tokens.cend();

    while (ptr < end) {
        root.program_level_statements.emplace_back(pm::parse_program_level_stmt(ptr, end));
    }

    return root;
}