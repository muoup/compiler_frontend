#include "program.h"

#include "statement.h"
#include "../util_methods.h"
#include "../../lexer/lex.h"

using namespace ast;

ast_node pm::parse_method_params(lex_cptr& ptr, const lex_cptr end) {
    return ast_node {
        .type = ast_node_type::PARAM_LIST,
        .children = parse_split(ptr, end, ",", parse_initialization)
    };
}

ast_node pm::parse_call_params(lex_cptr& ptr, const lex_cptr end) {
    auto call_params = ast_node { ast_node_type::PARAM_LIST };

    call_params.add_children(
        parse_split(ptr, end, ",", parse_expression)
    );

    return call_params;
}

ast_node pm::parse_method(lex_cptr& ptr, const lex_cptr) {
    const lex_cptr method_ret_type = assert_token(ptr, is_variable_identifier);
    const lex_cptr method_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER);

    ast_node method = {
        ast_node_type::FUNCTION,
        method_ret_type->span,
        method_name->span
    };

    method.add_child(
        parse_between(ptr, "(", parse_method_params)
    );

    method.add_child(
        parse_between(ptr, "{", parse_body)
    );

    return method;
}

ast_node pm::parse_body(lex_cptr &ptr, lex_cptr end) {
    return {};
}
