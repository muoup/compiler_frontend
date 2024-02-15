#include "analyzer.h"

#include <format>
#include "../lexer/lex.h"
#include "parser_methods.h"

using namespace ast;

ast_node ast::parse(const std::vector<lex::lex_token> &tokens) {
    ast_node root { ast_node_type::ROOT };

    auto ptr = tokens.cbegin();

    while (ptr < tokens.cend()) {
        parse_method(ptr, tokens.cend());
    }

    // for (auto ptr = tokens.begin(); ptr != tokens.end(); ++ptr) {
    //     const auto method_ret_type = assert_token_types(ptr, VARIABLE_IDENTIFIER);
    //     const auto method_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER);
    //
    //     const auto params = assert_token_val(ptr, "(");
    //
    //     if (params->closer == std::nullopt)
    //         throw_unclosed(*params);
    //
    //     ast_node method = root.add_child({
    //         ast_node_type::FUNCTION,
    //         method_ret_type->span,
    //         method_name->span
    //     });
    //
    //     method.add_child(
    //         parse_method_params(ptr, params->closer.value())
    //     );
    //
    //     assert_token_val(ptr, ")");
    //
    //     const auto body = assert_token_val(ptr, "{");
    //
    //     method.add_child(
    //         parse_body(ptr, body->closer.value())
    //     );
    //
    //     assert_token_val(ptr, "}");
    // }

    return root;
}