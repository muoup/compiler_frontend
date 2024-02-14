#include "analyzer.h"

#include <format>
#include "../lexer/lex.h"
#include "analyze_util.h"

using namespace ast;

const auto VARIABLE_IDENTIFIER = std::vector<lex::lex_type> {
    lex::lex_type::IDENTIFIER,
    lex::lex_type::KEYWORD
};

ast_node parse_conditional(lex_cptr& ptr, const lex_cptr end) {

}

ast_node parse_assignment(lex_cptr& ptr, const lex_cptr end) {

}

// Two variations:
// if ([condition]) { ... }
// [ident] = [expr];
ast_node parse_body(lex_cptr& ptr, const lex_cptr end) {
    ast_node body { ast_node_type::FUNCTION_BODY };

    while (ptr < end) {
        if (ptr->span == "if") {
            ++ptr;

            const auto open = assert_token_val(ptr, "(");

            body.add_child(parse_conditional(ptr, open->closer.value()));

            assert_token_val(ptr, ")");

            const auto body_open = assert_token_val(ptr, "{");

            body.add_child(parse_body(ptr, body_open->closer.value()));

            assert_token_val(ptr, "}");

        } else if (ptr->type == lex::lex_type::IDENTIFIER
                || ptr->type == lex::lex_type::KEYWORD) {

        }
    }
}

ast_node parse_method_params(lex_cptr& ptr, const lex_cptr end) {
    auto method_params = ast_node { ast_node_type::PARAM_LIST };

    while (ptr < end) {
        const auto param_type = assert_token_types(ptr, VARIABLE_IDENTIFIER);
        const auto param_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER);

        method_params.children.push_back(ast_node {
            ast_node_type::PARAM, param_type->span, param_name->span
        });

        if (ptr != end)
            assert_token_val(ptr, ",");
    }

    return method_params;
}

ast_node ast::parse(const std::vector<lex::lex_token> &tokens) {
    ast_node root { ast_node_type::ROOT };

    for (auto ptr = tokens.begin(); ptr != tokens.end(); ++ptr) {
        const auto method_ret_type = assert_token_types(ptr, VARIABLE_IDENTIFIER);
        const auto method_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER);

        const auto params = assert_token_val(ptr, "(");

        if (params->closer == std::nullopt)
            throw_unclosed(*params);

        ast_node method = root.add_child({
            ast_node_type::FUNCTION,
            method_ret_type->span,
            method_name->span
        });

        method.add_child(
            parse_method_params(ptr, params->closer.value())
        );

        assert_token_val(ptr, ")");

        const auto body = assert_token_val(ptr, "{");

        method.add_child(
            parse_body(ptr, body->closer.value())
        );

        assert_token_val(ptr, "}");
    }

    return root;
}