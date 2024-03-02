#pragma once
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace lex {
    struct lex_token;
    enum class lex_type;
}

namespace ast {
    enum class ast_node_type {
        ROOT,

        FUNCTION,
        PARAM_LIST,
        PARAM,
        CODE_BLOCK,

        CONDITIONAL,
        ASSIGNMENT,
        EXPRESSION,
        METHOD_CALL,

        TYPE,
        VARIABLE,
        LITERAL,
        OPERATOR
    };

    struct ast_node {
        ast_node_type type;
        std::string_view data {};
        std::string_view metadata {};

        std::vector<ast_node> children;

        // ast_node& add_child(ast_node&& child);
        ast_node& add_child(ast_node child);
        ast_node& add_child(ast_node_type type, std::string_view metadata = "", std::string_view data = "");

        template<class ... Args>
        void add_children(ast_node child, Args... children);
        void add_children(std::vector<ast_node> children);

        ast_node with_data(std::string_view data);
        ast_node with_metadata(std::string_view metadata);
    };

    using lex_cptr = std::vector<lex::lex_token>::const_iterator;
    using lex_ptr = std::vector<lex::lex_token>::iterator;

    using parse_fn = ast_node(*)(lex_cptr&, lex_cptr);
    using loop_fn = std::optional<ast_node>(*)(lex_cptr&);
    using parse_pred = bool(*)(lex_cptr);
}
