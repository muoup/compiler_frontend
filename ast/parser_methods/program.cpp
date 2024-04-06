#include "program.h"

#include "expression.h"
#include "../util_methods.h"
#include "../../lexer/lex.h"
#include "statement.h"

using namespace ast;

std::vector<nodes::type_instance> pm::parse_method_params(lex_cptr &ptr, const lex_cptr end) {
    return parse_split(ptr, end, ",", parse_type_instance);
}

std::vector<std::unique_ptr<nodes::expression>> pm::parse_call_params(lex_cptr &ptr, const lex_cptr end) {
    return parse_split(ptr, end, ",", parse_expression);
}

nodes::function pm::parse_method(lex_cptr &ptr, const lex_cptr end) {
    const auto ret_type = parse_value_type(ptr, end);
    const auto function_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;

    nodes::function function {
        ret_type,
        function_name,
        parse_between(ptr, "(", parse_method_params),
        parse_between(ptr, "{", parse_body)
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
    nodes::scope_block body;

    while (ptr != end)
        body.statements.emplace_back(parse_statement(ptr, end));

    return body;
}

nodes::value_type pm::parse_value_type(lex_cptr &ptr, const lex_cptr) {
    const auto is_const = !test_token_val(ptr, "mut").has_value();
    const auto type = assert_token(ptr, is_variable_identifier)->span;
    const auto is_pointer = test_token_val(ptr, "*").has_value();

    const auto intrin = nodes::get_intrinsic_type(type);

    if (!intrin) {
        return nodes::value_type {
            type,
            is_const,
            is_pointer,
        };
    } else {
        return nodes::value_type {
            *intrin,
            is_const,
            is_pointer,
        };
    }
}
