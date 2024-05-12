#include "program.h"

#include "expression.h"
#include "../util.h"
#include "../../lexer/lex.h"
#include "statement.h"
#include "../data/data_maps.h"

using namespace ast;

std::unique_ptr<nodes::program_level_stmt> pm::parse_program_level_stmt(ast::lex_cptr &ptr, ast::lex_cptr end) {
    if (ptr->span == "fn") {
        return std::make_unique<nodes::function>(parse_function(ptr, end));
    } else if (ptr->span == "struct") {
        return std::make_unique<nodes::struct_declaration>(parse_struct_decl(ptr, end));
    }

    return nullptr;
}

std::vector<nodes::type_instance> pm::parse_method_params(lex_cptr &ptr, const lex_cptr end) {
    return parse_split(ptr, end, ",", parse_type_instance);
}

std::vector<std::unique_ptr<nodes::expression>> pm::parse_call_params(lex_cptr &ptr, const lex_cptr end) {
    return parse_split(ptr, end, ",", parse_expr_tree);
}

nodes::function pm::parse_function(lex_cptr &ptr, const lex_cptr end) {
    assert_token_val(ptr, "fn");

    scope_stack.emplace_back();

    const auto function_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;
    auto params = parse_between(ptr, "(", parse_method_params);

    auto ret_type = test_token_val(ptr, "->") ?
                    parse_var_type(ptr, end) :
                    nodes::variable_type::void_type();

    function_types.emplace(function_name, ret_type);

    nodes::function function {
        ret_type,
        function_name,
        std::move(params),
        parse_body(ptr, end)
    };

    for (const auto &param : function.param_types) {
        scope_stack.back().emplace(param.var_name, param.type);
    }

    auto &code_expressions = function.body.statements;
    if (code_expressions.empty() || dynamic_cast<nodes::return_op*>(code_expressions.back().get()) == nullptr) {
        if (function_name == "main") {
            code_expressions.emplace_back(
                    std::make_unique<nodes::return_op>(
                            std::make_unique<nodes::literal>(0)
                    )
            );
        } else {
            code_expressions.emplace_back(std::make_unique<nodes::return_op>(nullptr));
        }
    }

    scope_stack.pop_back();

    return function;
}

nodes::scope_block pm::parse_body(lex_cptr &ptr, lex_cptr end) {
    nodes::scope_block::scope_stmts stmts;

    scope_stack.emplace_back();

    if (test_token_val(ptr, "{")) {
        while (!test_token_val(ptr, "}"))
            stmts.emplace_back(parse_statement(ptr, end));
    } else {
        stmts.emplace_back(parse_statement(ptr, end));
    }

    scope_stack.pop_back();

    return nodes::scope_block { std::move(stmts) };
}

nodes::variable_type pm::parse_var_type(lex_cptr &ptr, lex_cptr) {
    const auto is_const = !test_token_val(ptr, "mut").has_value();
    const auto is_volatile = test_token_val(ptr, "volatile").has_value();
    const auto type = assert_token(ptr, is_variable_identifier)->span;
    uint8_t pointer_count = 0;

    while (test_token_val(ptr, "*"))
        pointer_count++;

    if (auto intrinsicType = find_element(ast::pm::intrin_map, type)) {
        return nodes::variable_type {
            *intrinsicType,
            is_const,
            is_volatile,
            pointer_count
        };
    } else {
        return nodes::variable_type {
            type,
            is_const,
            is_volatile,
            pointer_count
        };
    }
}
