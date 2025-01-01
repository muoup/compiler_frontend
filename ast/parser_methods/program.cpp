#include "program.h"

#include <charconv>

#include "expression.h"
#include "../../lexer/lex.h"
#include "statement.h"
#include "declarations.h"

using namespace ast;

std::unique_ptr<nodes::program_level_stmt> pm::parse_program_level_stmt(ast::lex_cptr &ptr, ast::lex_cptr end) {
    if (test_token_val(ptr, "_libc") || peek(ptr, end)->span == "fn") {
        return parse_function_prototype(ptr, end);
    } else if (peek(ptr, end)->span == "struct") {
        return parse_struct_decl(ptr, end);
    } else if (test_token_val(ptr, ";")) {
        return nullptr;
    }

    throw std::runtime_error("Unknown program level statement");
}

ast::nodes::method_params pm::parse_method_params(lex_cptr &ptr, const lex_cptr end) {
    ast::nodes::method_params method_params;

    for (auto param : parse_split_type_inst(ptr, end)) {
        if (param.var_name == "...") {
            method_params.is_var_args = true;
            break;
        }

        method_params.data.emplace_back(std::move(param));
    }

    return method_params;
}

std::vector<std::unique_ptr<nodes::expression>> pm::parse_expression_list(lex_cptr &ptr, const lex_cptr end) {
    return parse_split(ptr, end, ",", parse_expr_tree);
}

std::unique_ptr<nodes::function_prototype> pm::parse_function_prototype(ast::lex_cptr &ptr, ast::lex_cptr end) {
    assert_token_val(ptr, "fn");

    const auto function_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;
    auto params = parse_between(ptr, "(", parse_method_params);

    auto ret_type = test_token_val(ptr, "->") ?
                    parse_var_type(ptr, end) :
                    nodes::variable_type::void_type();

    auto prototype = std::make_unique<nodes::function_prototype>(
        ret_type,
        function_name,
        std::move(params),
        nullptr
    );

    function_prototypes.emplace(function_name, prototype.get());

    if (!test_token_val(ptr, ";")) {
        prototype->implementation = std::make_unique<nodes::function>(
                parse_function(ptr, end, prototype.get())
        );
    }

    return prototype;
}

nodes::function pm::parse_function(lex_cptr &ptr, const lex_cptr end, const nodes::function_prototype *prototype) {
    scope_stack.emplace_back();
    current_function = prototype;

    for (const auto &param : prototype->params.data)
        scope_stack.back().emplace(param.instance.var_name, param.instance.type);

    auto body = parse_body(ptr, end);
    auto &code_expressions = body.statements;

    if (code_expressions.empty() || dynamic_cast<nodes::return_op*>(code_expressions.back().get()) == nullptr) {
        if (prototype->fn_name == "main" && prototype->return_type != nodes::variable_type::void_type()) {
            code_expressions.emplace_back(
                std::make_unique<nodes::return_op>(
                std::make_unique<nodes::literal>(0)
                )
            );
        } else {
            code_expressions.emplace_back(std::make_unique<nodes::return_op>(nullptr));
        }
    }

    current_function = nullptr;
    scope_stack.pop_back();

    return nodes::function {
        std::move(body),
        prototype
    };
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

nodes::variable_type pm::parse_var_type(lex_cptr &ptr, lex_cptr end) {
    const auto is_volatile = test_token_val(ptr, "volatile").has_value();
    const auto is_const = !test_token_val(ptr, "mut").has_value();
    const auto type = assert_token(ptr, is_variable_identifier)->span;
    int array_length = 0;
    uint8_t pointer_count = 0;

    if (test_token_val(ptr, "[")) {
        if (auto len = test_token_type(ptr, lex::lex_type::INT_LITERAL))
            std::from_chars((*len)->span.data(), (*len)->span.data() + (*len)->span.size(), array_length);
        else
            array_length = -1;

        pointer_count++;

        assert_token_val(ptr, "]");
    }

    while (test_token_val(ptr, "*"))
        pointer_count++;

    if (auto intrinsicType = find_element(ast::pm::intrin_map, type)) {
        return nodes::variable_type {
            *intrinsicType,
            is_const,
            is_volatile,
            pointer_count,
            array_length
        };
    } else {
        return nodes::variable_type {
            type,
            is_const,
            is_volatile,
            pointer_count,
            array_length
        };
    }
}
