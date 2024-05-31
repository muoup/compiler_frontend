#pragma once

#include "../data/ast_nodes.h"

namespace ast::val {
    void validate(ast::nodes::root& root);

    static void cache_function(ast::nodes::function_prototype *func);
    static void cache_struct(ast::nodes::struct_declaration *func);

    static void validate_function(ast::nodes::function_prototype *func);
    static void validate_statement(ast::nodes::statement *stmt);

    static void validate_block(ast::nodes::scope_block *block);

    static void validate_expression(std::unique_ptr<ast::nodes::expression> &tree);

    static void validate_method_call(ast::nodes::method_call *call);
    static void validate_bin_op(ast::nodes::bin_op *op);
    static void validate_un_op(ast::nodes::un_op *op);

    static void validate_assn(ast::nodes::assignment *assn);
    static void validate_access(ast::nodes::bin_op *access);

    static void cast_to(std::unique_ptr<ast::nodes::expression> &expr, ast::nodes::variable_type type);
    static void create_load(std::unique_ptr<ast::nodes::expression> &expr);

    static std::optional<nodes::variable_type> find_variable(std::string_view name);
}
