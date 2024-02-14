#pragma once

#include <span>
#include <string_view>
#include <vector>

namespace lex {
    struct lex_token;
}

namespace ast {
    enum class ast_node_type {
        ROOT,

        FUNCTION,
        PARAM_LIST,
        PARAM,
        FUNCTION_BODY,

        CONDITIONAL,

        TYPE,
        VARIABLE,
        LITERAL,
        OPERATOR
    };

    struct ast_node {
        ast_node_type type;
        std::string_view metadata {};
        std::string_view data {};

        std::vector<ast_node> children;

        ast_node& add_child(ast_node&& child) {
            children.push_back(child);
            return children.back();
        }
    };

    ast_node parse(const std::vector<lex::lex_token> &tokens);
}
