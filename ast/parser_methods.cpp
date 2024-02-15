#include "parser_methods.h"
#include "../lexer/lex.h"
#include "analyzer.h"

using namespace ast;

const auto VARIABLE_IDENTIFIER = std::vector<lex::lex_type> {
    lex::lex_type::IDENTIFIER,
    lex::lex_type::KEYWORD
};

ast_node ast::parse_conditional(lex_cptr& ptr, const lex_cptr end) {

}

ast_node ast::parse_assignment(lex_cptr& ptr, const lex_cptr end) {

}

// Two variations:
// if ([condition]) { ... }
// [ident] = [expr];
ast_node ast::parse_body(lex_cptr& ptr, const lex_cptr end) {
    ast_node body { ast_node_type::FUNCTION_BODY };

    while (ptr < end) {
        if (test_token_val(ptr, "if")) {

            auto conditional = body.add_child(
                ast_node { ast_node_type::CONDITIONAL }
            );

            conditional.add_child(
                parse_between(ptr = assert_token_val(ptr, "("), parse_conditional)
            );

            conditional.add_child(
                parse_between(ptr = assert_token_val(ptr, "{"), parse_body)
            );

        } else if (test_token_type(ptr, VARIABLE_IDENTIFIER)) {
            auto assignment = body.add_child(
                ast_node { ast_node_type::ASSIGNMENT, ptr->span }
            );


        }
    }

    return body;
}

ast_node ast::parse_method_params(lex_cptr& ptr, const lex_cptr end) {
    auto method_params = ast_node { ast_node_type::PARAM_LIST };

    while (ptr < end) {
        const auto param_type = assert_token_type(ptr, VARIABLE_IDENTIFIER);
        const auto param_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER);

        method_params.children.push_back(ast_node {
            ast_node_type::PARAM, param_type->span, param_name->span
        });

        if (ptr != end)
            assert_token_val(ptr, ",");
    }

    return method_params;
}

ast_node ast::parse_method(lex_cptr& ptr, const lex_cptr end) {
    const auto method_ret_type = assert_token_type(ptr, VARIABLE_IDENTIFIER);
    const auto method_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER);

    ast_node method = {
        ast_node_type::FUNCTION,
        method_ret_type->span,
        method_name->span
    };

    method.add_child(
        parse_between(ptr = assert_token_val(ptr, "("), parse_method_params)
    );

    method.add_child(
        parse_between(ptr = assert_token_val(ptr, "{"), parse_body)
    );

    return method;
}

ast_node ast::parse_between(lex_cptr& ptr, const parse_fn fn) {
    const auto end = ptr->closer.value();

    return fn(++ptr, end);
}
