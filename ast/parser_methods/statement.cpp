#include "statement.h"

#include <charconv>
#include <stack>
#include <stdexcept>

#include "operator.h"
#include "../declarations.h"
#include "../util_methods.h"
#include "../../lexer/lex.h"
#include "../../lexer/literal_cache.h"

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
    if (ptr->type == lex::lex_type::KEYWORD
        || ptr->type == lex::lex_type::IDENTIFIER && (ptr + 1)->type == lex::lex_type::IDENTIFIER)
        return parse_initialization(ptr, end);

    if (ptr->type == lex::lex_type::IDENTIFIER) {
        if ((ptr + 1)->span == "(") {
            return ast_node {
                ast_node_type::METHOD_CALL,
                "",
                "",
                parse_split(++ptr, ptr->closer.value(), ",", parse_expression)
            };
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

// void pm::try_optimize(ast_node& node) {
//     for (auto& child : node.children) {
//         try_optimize(child);
//     }
//
//     if (node.children.size() != 2)
//         return;
//
//     const auto& child1 = node.children[0];
//     const auto& child2 = node.children[1];
//
//     if (child1.type == ast_node_type::LITERAL && child2.type == ast_node_type::LITERAL) {
//         const auto& data1 = child1.data;
//         const auto& data2 = child2.data;
//
//         int num1, num2;
//         std::from_chars(data1.data(), data1.data() + data1.size(), num1);
//         std::from_chars(data2.data(), data2.data() + data2.size(), num2);
//
//         lex::literals.push_back(
//             std::to_string(
//                 pm::binop_fn_map.at(node.data)(num1, num2)
//                 )
//             );
//
//         node = ast_node {
//             ast_node_type::LITERAL,
//             "",
//             lex::literals.back()
//         };
//     }
// }