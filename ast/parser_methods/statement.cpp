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

nodes::initialization pm::parse_initialization(lex_cptr &ptr, lex_cptr end) {
    const auto type = parse_value_type(ptr, end);
    const auto var_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;

    return nodes::initialization {
        type,
        nodes::variable { var_name }
    };
}

nodes::un_op pm::parse_unop(lex_cptr &ptr, const lex_cptr end) {
    const auto operator_type = assert_token_type(ptr, lex::lex_type::SYMBOL)->span;
    auto expr = parse_value(++ptr, end);

    if (!expr)
        throw std::runtime_error("Expected value after operator");

    ++ptr;


    return { nodes::un_op {
        get_unop(operator_type),
        std::make_unique<nodes::expression>(std::move(expr.value()))
    } };
}

nodes::bin_op pm::parse_binop(lex_cptr &ptr, const lex_cptr end) {
    const auto operator_type = assert_token_type(ptr, lex::lex_type::SYMBOL)->span;
    const auto op_slice = operator_type.ends_with("=") ?
        operator_type.substr(0, operator_type.size() - 1) : operator_type;

    return nodes::bin_op {
        nodes::bin_op {
        get_binop(op_slice),
        operator_type.ends_with("="),
        }
    };
}

nodes::expression pm::parse_expression(lex_cptr &ptr, const lex_cptr end) {
    std::stack<nodes::expression> expr_stack;
    std::stack<nodes::bin_op> binop_stack;

    const auto gen_expr = [&ptr, end] {
        return parse_value(ptr, end).or_else([&ptr, end] {
            if (ptr >= end - 1)
                throw std::runtime_error("Expected value");

            return std::optional {
                nodes::expression {
                parse_unop(ptr, end)
                }
            };
        }).value();
    };

    const auto parse_assignment = [&expr_stack, &binop_stack] {
        auto right = std::make_unique<nodes::expression>(std::move(expr_stack.top()));
        expr_stack.pop();
        auto left = std::move(expr_stack.top());
        expr_stack.pop();
        auto binop_type = binop_stack.top().type;
        binop_stack.pop();

        switch (left.value.index()) {
            case nodes::expression_type::VARIABLE:
                expr_stack.emplace(nodes::assignment {
                    std::get<nodes::variable>(left.value),
                    std::move(right),
                    binop_type
                });
                break;
            case nodes::expression_type::INITIALIZATION:
                expr_stack.emplace(nodes::assignment {
                    std::get<nodes::initialization>(left.value),
                    std::move(right),
                    binop_type
                });
                break;
            default:
                throw std::runtime_error("Invalid assignment target");
        }
    };

    const auto combine_back = [&expr_stack, &binop_stack, &parse_assignment] {
        auto& top_op = binop_stack.top();

        if (top_op.assignment) {
            parse_assignment();
            return;
        }

        top_op.right = std::make_unique<nodes::expression>(std::move(expr_stack.top()));
        expr_stack.pop();

        top_op.left = std::make_unique<nodes::expression>(std::move(expr_stack.top()));
        expr_stack.pop();

        expr_stack.emplace(std::move(top_op));
        binop_stack.pop();
    };

    expr_stack.emplace(gen_expr());

    while (ptr < end) {
        auto r_op = parse_binop(ptr, end);

        while (!binop_stack.empty() && get_prec(r_op) <= get_prec(binop_stack.top()))
            combine_back();

        expr_stack.emplace(gen_expr());
        binop_stack.emplace(std::move(r_op));
    }

    while (!binop_stack.empty())
        combine_back();

    // try_optimize(expr_stack.top());

    return nodes::expression {
        std::move(expr_stack.top())
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
        return nodes::expression { nodes::variable { ptr++->span } };

    return std::nullopt;
}

std::optional<nodes::literal> pm::parse_literal(lex_cptr &ptr, lex_cptr end) {
    int i = 0;
    double d = 0;

    switch (ptr->type) {
        case lex::lex_type::INT_LITERAL:
            std::from_chars(ptr->span.data(), ptr->span.data() + ptr++->span.size(), i);
            return nodes::literal { i };
        case lex::lex_type::FLOAT_LITERAL:
            std::from_chars(ptr->span.data(), ptr->span.data() + ptr++->span.size(), d);
            return nodes::literal { d };
        case lex::lex_type::STRING_LITERAL:
            return nodes::literal { ptr++->span };
        case lex::lex_type::CHAR_LITERAL:
            return nodes::literal { ptr++->span.front() };
        default:
            return std::nullopt;
    }
}
