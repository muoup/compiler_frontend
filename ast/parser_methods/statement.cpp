#include "statement.h"

#include <charconv>
#include <stack>
#include <stdexcept>

#include "operator.h"
#include "../declarations.h"
#include "../util_methods.h"
#include "../../lexer/lex.h"

using namespace ast;

ast_node pm::parse_statement(lex_cptr &ptr, lex_cptr end) {
    if (ptr->span == "if") {
        // return parse_conditional(ptr, end);
        return {};
    } else if (ptr->span == "return") {
        return ast_node {
            ast_node_type::RETURN,
            "",
            "",
            { parse_until(++ptr, end, ";", parse_expression) }
        };
    }

    return parse_until(ptr, end, ";", parse_expression);
}

ast_node pm::parse_initialization(lex_cptr &ptr, lex_cptr end) {
    const auto metadata = ptr++->span;
    const auto data = ptr++->span;

    return ast_node {
        ast_node_type::INITIALIZATION,
        data,
        metadata
    };
}

ast_node combine_expr(const ast_node expr1, ast_node op, const ast_node expr2) {
    op.add_child(expr1);
    op.add_child(expr2);
    return op;
}

ast_node pm::parse_expression(lex_cptr &ptr, const lex_cptr end) {
    std::stack<ast_node> expr_stack;
    std::stack<ast_node> op_stack;

    ast_node expr_root {
        ast_node_type::EXPRESSION
    };

    const auto gen_expr = [&ptr, end] {
        auto expr = parse_value(ptr, end);

        if (expr)
            return expr.value();

        ast_node op {
            ast_node_type::UN_OP,
            assert_token_type(ptr, lex::lex_type::SYMBOL)->span
        };

        expr = parse_value(ptr, end);

        if (!expr)
            throw std::runtime_error("Expected expression after operator.");

        op.add_child(expr.value());
        return op;
    };

    const auto combine_back = [&expr_stack, &op_stack] {
        auto top_expr = std::move(expr_stack.top());
        expr_stack.pop();

        auto top_op = std::move(op_stack.top());
        op_stack.pop();

        auto next_expr = std::move(expr_stack.top());
        expr_stack.pop();

        expr_stack.emplace(
            combine_expr(std::move(next_expr), std::move(top_op), std::move(top_expr))
        );
    };

    expr_stack.emplace(gen_expr());

    while (ptr != end) {
        auto op = ast_node {
            ast_node_type::BIN_OP,
            assert_token_type(ptr, lex::lex_type::SYMBOL)->span
        };

        while (!op_stack.empty() && !greater_prec(op.data, op_stack.top().data))
            combine_back();

        expr_stack.emplace(gen_expr());
        op_stack.emplace(std::move(op));
    }

    while (!op_stack.empty())
        combine_back();

    // try_optimize(expr_stack.top());

    expr_root.add_child(std::move(expr_stack.top()));
    return expr_root;
}

std::optional<ast_node> pm::parse_value(lex_cptr &ptr, const lex_cptr end) {
    if (ptr->type == lex::lex_type::KEYWORD || ptr->type == lex::lex_type::PRIMITIVE)
        return parse_initialization(ptr, end);

    if (ptr->type == lex::lex_type::IDENTIFIER) {
        if ((ptr + 1)->span == "(") {
            ast_node method_call {
                ast_node_type::METHOD_CALL,
                ptr++->span,
                ""
            };

            const auto param_end = ptr++->closer.value();

            ast_node arg_list {
                ast_node_type::PARAM_LIST,
                "",
                "",
                parse_split(ptr, param_end, ",", parse_expression)
            };

            method_call.add_child(std::move(arg_list));
            ptr = param_end + 1;
            return method_call;
        }

        return ast_node {
            ast_node_type::VARIABLE,
            ptr++->span,
            ""
        };
    }
    else if (lex::LITERAL_MAP.contains(ptr->type)) {
        return ast_node {
            lex::LITERAL_MAP.at(ptr->type),
            ptr++->span,
            ""
        };
    } else if (ptr->span == "(") {
        return std::move(parse_between(ptr, parse_expression).children[0]);
    }

    return std::nullopt;
}