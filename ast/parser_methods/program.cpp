#include "program.h"

#include "expression.h"
#include "../util.h"
#include "../../lexer/lex.h"
#include "statement.h"

using namespace ast;

std::unique_ptr<nodes::program_level_stmt> pm::parse_program_level_stmt(ast::lex_cptr &ptr, ast::lex_cptr end) {
    if (ptr->span == "fn") {
        return std::make_unique<nodes::function>(parse_method(ptr, end));
    } else if (ptr->span == "struct") {
        return std::make_unique<nodes::struct_declaration>(parse_struct_decl(ptr, end));
    }

    return nullptr;
}

std::vector<nodes::type_instance> pm::parse_method_params(lex_cptr &ptr, const lex_cptr end) {
    return parse_split(ptr, end, ",", parse_type_instance);
}

std::vector<std::unique_ptr<nodes::expression>> pm::parse_call_params(lex_cptr &ptr, const lex_cptr end) {
    return parse_split(ptr, end, ",", parse_expression);
}

nodes::function pm::parse_method(lex_cptr &ptr, const lex_cptr end) {
    assert_token_val(ptr, "fn");

    const auto function_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;
    auto params = parse_between(ptr, "(", parse_method_params);

    auto ret_type = test_token_val(ptr, "->") ?
            parse_value_type(ptr, end) :
            nodes::value_type { "void" };

    nodes::function function {
        ret_type,
        function_name,
        std::move(params),
        parse_body(ptr, end)
    };

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

    return function;
}

nodes::scope_block pm::parse_body(lex_cptr &ptr, lex_cptr end) {
    nodes::scope_block::scope_stmts stmts;

    if (test_token_val(ptr, "{")) {
        while (!test_token_val(ptr, "}"))
            stmts.emplace_back(parse_statement(ptr, end));
    } else {
        stmts.emplace_back(parse_statement(ptr, end));
    }

    return nodes::scope_block { std::move(stmts) };
}

nodes::value_type pm::parse_value_type(lex_cptr &ptr, const lex_cptr) {
    const auto is_const = !test_token_val(ptr, "mut").has_value();
    const auto is_volatile = test_token_val(ptr, "volatile").has_value();
    const auto type = assert_token(ptr, is_variable_identifier)->span;
    const auto is_pointer = test_token_val(ptr, "*").has_value();

    auto instrinsic = nodes::get_intrinsic_type(type);

    if (!instrinsic) {
        return nodes::value_type {
            type,
            is_const,
            is_pointer,
            is_volatile
        };
    } else {
        return nodes::value_type {
            *instrinsic,
            is_const,
            is_pointer,
            is_volatile
        };
    }
}
