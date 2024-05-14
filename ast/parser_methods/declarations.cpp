#include "declarations.h"

#include "statement.h"
#include "../../lexer/lex.h"
#include "expression.h"

using namespace ast;

std::vector<nodes::type_instance> pm::parse_split_type_inst(lex_cptr &ptr, const lex_cptr end) {
    return parse_split(ptr, end, ",", pm::parse_type_instance);
}

std::unique_ptr<nodes::struct_declaration> pm::parse_struct_decl(lex_cptr &ptr, const lex_cptr end) {
    assert_token_val(ptr, "struct");

    scope_stack.emplace_back();

    auto name = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;
    auto types = parse_between(ptr, "{", parse_split_type_inst);

    struct_types.emplace(name, types);
    scope_stack.pop_back();

    return std::make_unique<nodes::struct_declaration>(name, std::move(types));
}