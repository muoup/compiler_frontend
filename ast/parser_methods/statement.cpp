#include "statement.h"

#include <charconv>
#include <stack>
#include <stdexcept>

#include "operator.h"
#include "program.h"
#include "../declarations.h"
#include "../util_methods.h"
#include "../../lexer/lex.h"

using namespace ast;

nodes::statement pm::parse_statement(lex_cptr &ptr, lex_cptr end) {
    if (ptr->span == "if" || ptr->span == "while" || ptr->span == "do") {
        return { parse_conditional(ptr, end) };
    } else if (ptr->span == "return") {
        auto expr = parse_expression(++ptr, end);

        return { std::move(expr) };
    }

    return nodes::statement {
        parse_until(ptr, end, ";", parse_expression)
    };
}

nodes::conditional pm::parse_conditional(lex_cptr &ptr, lex_cptr end) {
    nodes::conditional_type type;

    if (ptr->span == "if")
        type = nodes::conditional_type::IF_STATEMENT;
    else if (ptr->span == "while")
        type = nodes::conditional_type::WHILE_LOOP;
    else
        type = nodes::conditional_type::DO_WHILE_LOOP;

    return nodes::conditional {
        .type = type,
        .condition = parse_between(++ptr, parse_expression),
        .body = parse_between(ptr, parse_body)
    };
}

nodes::initialization pm::parse_initialization(lex_cptr &ptr, lex_cptr end) {
    auto type = parse_value_type(ptr, end);
    auto var_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;

    return nodes::initialization {
        type,
        nodes::variable { var_name }
    };
}

nodes::expression pm::parse_expression(lex_cptr &ptr, const lex_cptr end) {
    std::stack<nodes::expression> expr_stack;
    std::stack<nodes::op> op_stack;

    const auto gen_expr = [&ptr, end] {
        if (auto expr = parse_value(ptr, end))
            return expr.value();

        const auto& operator_type = assert_token_type(ptr, lex::lex_type::SYMBOL)->span;

        nodes::op op {
            get_op(operator_type),
        };

        auto val = parse_value(++ptr, end);

        if (!val)
            throw std::runtime_error("Expected value after operator");

        op.operands.emplace_back(std::move(val.value()));

        return nodes::expression { std::move(op) };
    };

    const auto combine_back = [&expr_stack, &op_stack] {
        auto& top_op = op_stack.top();

        top_op.operands.emplace_back(std::move(expr_stack.top()));
        expr_stack.pop();

        top_op.operands.emplace_back(std::move(expr_stack.top()));
        expr_stack.pop();

        expr_stack.emplace(std::move(top_op));
        op_stack.pop();
    };

    expr_stack.emplace(gen_expr());

    while (ptr != end) {
        auto op = nodes::op {
            .type = get_op(ptr->span)
        };

        while (!op_stack.empty() && get_prec(op) < get_prec(op_stack.top()))
            combine_back();

        expr_stack.emplace(gen_expr());
        op_stack.emplace(std::move(op));
    }

    while (!op_stack.empty())
        combine_back();

    // try_optimize(expr_stack.top());

    return nodes::expression {
        std::move(expr_stack.top())
    };
}

std::optional<nodes::expression> pm::parse_value(lex_cptr &ptr, const lex_cptr end) {
    if (is_variable_identifier(ptr))
        return nodes::expression { parse_initialization(ptr, end) };

    if (ptr->type == lex::lex_type::IDENTIFIER) {
        if ((ptr + 1)->span == "(") {
            nodes::method_call method_call {
                ptr++->span,
                {}
            };

            const auto param_end = ptr++->closer.value();

            method_call.arguments = parse_split(ptr, param_end, ",", parse_expression);

            ptr = param_end + 1;
            return nodes::expression { method_call };
        }

        return nodes::expression { nodes::variable { ptr++->span } };
    } else if (auto literal = parse_literal(ptr, end)){
        return nodes::expression { literal.value() };
    } else if (ptr->span == "(") {
        return parse_between(ptr, parse_expression);
    }

    return std::nullopt;
}

std::optional<nodes::literal> pm::parse_literal(lex_cptr &ptr, lex_cptr end) {
    if (!lex::LITERAL_SET.contains(ptr->type))
        return std::nullopt;

    switch (ptr->type) {
        case lex::lex_type::INT_LITERAL:
            int i;
            std::from_chars(ptr->span.data(), ptr->span.data() + ptr->span.size(), i);
            return nodes::literal { i };
        case lex::lex_type::FLOAT_LITERAL:
            double d;
            std::from_chars(ptr->span.data(), ptr->span.data() + ptr->span.size(), d);
            return nodes::literal { d };
        case lex::lex_type::STRING_LITERAL:
            return nodes::literal { ptr++->span };
        case lex::lex_type::CHAR_LITERAL:
            return nodes::literal { ptr++->span.front() };
        default:
            std::unreachable();
    }
}
