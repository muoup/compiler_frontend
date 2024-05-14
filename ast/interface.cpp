#include <iostream>

#include "interface.h"
#include "parser_methods/program.h"

using namespace ast;

nodes::root ast::parse(const std::vector<lex::lex_token> &tokens) {
    nodes::root root {};

    scope_stack.clear();
    scope_stack.emplace_back();

    struct_types.clear();

    function_prototypes.clear();
    current_function = nullptr;

    auto ptr = tokens.cbegin();
    const auto end = tokens.cend();

    while (ptr < end) {
        if (auto stmt = pm::parse_program_level_stmt(ptr, end); stmt != nullptr)
            root.program_level_statements.emplace_back(std::move(stmt));
    }

    return root;
}