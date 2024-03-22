#include "statement.h"

#include <charconv>
#include <stdexcept>

#include "expression.h"
#include "operator.h"
#include "program.h"
#include "../declarations.h"
#include "../util_methods.h"
#include "../../lexer/lex.h"

using namespace ast;

nodes::statement pm::parse_statement(lex_cptr &ptr, lex_cptr end) {
    if (ptr->span == "if" || ptr->span == "while" || ptr->span == "do") {
        return nodes::statement {
            parse_conditional(ptr, end)
        };
    } else if (ptr->span == "return") {
        return nodes::statement {
            nodes::return_op {
                ++ptr != end ?
                    std::optional {
                        parse_until(ptr, end, ";", parse_expression)
                    } : std::nullopt
            }
        };
    }

    return nodes::statement {
        parse_until(ptr, end, ";", parse_expression)
    };
}

nodes::conditional pm::parse_conditional(lex_cptr &ptr, lex_cptr end) {
    nodes::conditional_type cond_type;

    if (ptr->span == "if")
        cond_type = nodes::conditional_type::IF_STATEMENT;
    else if (ptr->span == "while")
        cond_type = nodes::conditional_type::WHILE_LOOP;
    else
        cond_type = nodes::conditional_type::DO_WHILE_LOOP;

    return nodes::conditional {
        .type = cond_type,
        .condition = parse_between(++ptr, parse_expression),
        .body = parse_between(ptr, parse_body)
    };
}

nodes::type_instance pm::parse_initialization(lex_cptr &ptr, lex_cptr end) {
    return nodes::type_instance {
        parse_value_type(ptr, end),
        assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span,
    };
}

nodes::un_op pm::parse_unop(lex_cptr &ptr, const lex_cptr end) {
    const auto operator_type = assert_token_type(ptr, lex::lex_type::SYMBOL)->span;
    auto expr = parse_value(++ptr, end);

    if (!expr)
        throw std::runtime_error("Expected value after operator");

    ++ptr;

    return nodes::un_op {
        get_unop(operator_type),
        std::make_unique<nodes::expression>(std::move(expr.value()))
    };
}

nodes::bin_op pm::parse_binop(lex_cptr &ptr, const lex_cptr end) {
    const auto operator_type = assert_token_type(ptr, lex::lex_type::SYMBOL)->span;
    const auto op_slice = operator_type.ends_with("=") ?
        operator_type.substr(0, operator_type.size() - 1) : operator_type;

    return nodes::bin_op {
        get_binop(op_slice),
        operator_type.ends_with("="),
    };
}

nodes::method_call pm::parse_method_call(lex_cptr &ptr, const lex_cptr end) {
    const auto method_name = ptr++->span;
    const auto param_end = ptr++->closer.value();

    nodes::method_call method_call {
        method_name,
        parse_split<nodes::expression, lex_cptr>(ptr, param_end, ",", parse_expression)
    };

    ptr = param_end + 1;

    return method_call;
}

std::optional<nodes::expression> pm::parse_value(lex_cptr &ptr, const lex_cptr end) {
    if (auto literal = parse_literal(ptr, end))
        return nodes::expression { literal.value() };

    if (ptr->span == "(")
        return parse_between(ptr, parse_expression);

    if (is_variable_identifier(ptr) && (ptr + 1)->type == lex::lex_type::IDENTIFIER)
        return nodes::expression { parse_initialization(ptr, end) };

    if (ptr->type == lex::lex_type::IDENTIFIER && (ptr + 1)->span == "(")
        return nodes::expression { parse_method_call(ptr, end) };

    if (ptr->type == lex::lex_type::IDENTIFIER)
        return nodes::expression { nodes::var_ref { ptr++->span } };

    return std::nullopt;
}

std::optional<nodes::literal> pm::parse_literal(lex_cptr &ptr, lex_cptr end) {
    int i = 0;
    double d = 0;

    switch (ptr->type) {
        case lex::lex_type::INT_LITERAL:
            std::from_chars(ptr->span.data(), ptr->span.data() + ptr->span.size(), i);
            ++ptr;
            return nodes::literal { i };
        case lex::lex_type::FLOAT_LITERAL:
            std::from_chars(ptr->span.data(), ptr->span.data() + ptr->span.size(), d);
            ++ptr;
            return nodes::literal { d };
        case lex::lex_type::STRING_LITERAL:
            return nodes::literal { ptr++->span };
        case lex::lex_type::CHAR_LITERAL:
            return nodes::literal { ptr++->span.front() };
        default:
            return std::nullopt;
    }
}
