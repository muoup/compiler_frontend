#include "declarations.h"

#include "statement.h"
#include "../../lexer/lex.h"

using namespace ast;

std::vector<nodes::type_instance> pm::parse_struct_types(lex_cptr &ptr, const lex_cptr end) {
    return parse_split(ptr, end, ";", parse_type_instance);
}

nodes::struct_declaration pm::parse_struct_decl(lex_cptr &ptr, const lex_cptr end) {
    assert_token_val(ptr, "struct");

    scope_stack.emplace_back();

    auto name = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;
    auto types = parse_between(ptr, "{", parse_struct_types);

    struct_types.emplace(name, types);
    scope_stack.pop_back();

    return nodes::struct_declaration { name, std::move(types) };
}