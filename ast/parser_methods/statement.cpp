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
    if (ptr->span.compare("if")) {
        // return parse_conditional(ptr, end);
        return {};
    } else {
        return parse_expression(ptr, end);
    }
}

ast_node pm::parse_initialization(lex_cptr &ptr, lex_cptr end) {
    return {};
}

ast_node& flush_operators(ast_node& curr_root, std::stack<lex_cptr> &ident_stack, std::stack<lex_cptr> &op_stack) {
    while (!op_stack.empty()) {
        const auto &op = curr_root.add_child(
            ast_node_type::OPERATOR, op_stack.top()->span
        );
        op_stack.pop();

        curr_root.add_child(
            pm::parse_value(ident_stack.top())
        );
        ident_stack.pop();

        curr_root = op;
    }

    return curr_root;
}

ast_node& add_operator(const lex_cptr ptr, ast_node& curr_root, std::stack<lex_cptr> &ident_stack, std::stack<lex_cptr> &op_stack) {
    if (op_stack.empty() || pm::binop_prec.at(op_stack.top()->span) >= pm::binop_prec.at(ptr->span)) {
        op_stack.push(ptr);
        return curr_root;
    }

    auto &binop = curr_root.add_child(ast_node {
        ast_node_type::OPERATOR,
        ptr->span
    });

    auto& lbottom = flush_operators(curr_root, ident_stack, op_stack);

    lbottom.add_child(pm::parse_value(ident_stack.top()));
    ident_stack.pop();

    return binop;
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

    const auto combine_back = [&expr_stack, &op_stack]() {
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

    expr_stack.emplace(parse_value(ptr));

    while (ptr != end) {
        auto op = ast_node {
            ast_node_type::OPERATOR,
            ptr++->span
        };

        auto expr = parse_value(ptr);

        while (!op_stack.empty() && !greater_prec(op.data, op_stack.top().data))
            combine_back();

        expr_stack.emplace(expr);
        op_stack.emplace(std::move(op));
    }

    while (!op_stack.empty())
        combine_back();

    try_optimize(expr_stack.top());

    expr_root.add_child(std::move(expr_stack.top()));
    return expr_root;
}

ast_node pm::parse_value(lex_cptr &ptr) {
    if (ptr->type == lex::lex_type::IDENTIFIER) {
        return ast_node {
            ast_node_type::VARIABLE,
            "",
            ptr++->span
        };
    } else if (lex::LITERAL_SET.contains(ptr->type)) {
        return ast_node {
            ast_node_type::LITERAL,
            "",
            ptr++->span
        };
    } else if (ptr->span == "(") {
        return std::move(parse_between(ptr, parse_expression).children[0]);
    }

    throw std::runtime_error("Invalid value");
}

void pm::try_optimize(ast_node& node) {
    for (auto& child : node.children) {
        try_optimize(child);
    }

    if (node.children.size() != 2)
        return;

    const auto& child1 = node.children[0];
    const auto& child2 = node.children[1];

    if (child1.type == ast_node_type::LITERAL && child2.type == ast_node_type::LITERAL) {
        const auto& data1 = child1.metadata;
        const auto& data2 = child2.metadata;

        int num1, num2;
        std::from_chars(data1.data(), data1.data() + data1.size(), num1);
        std::from_chars(data2.data(), data2.data() + data2.size(), num2);

        lex::literals.push_back(
            std::to_string(
                pm::binop_fn_map.at(node.data)(num1, num2)
                )
            );

        node = ast_node {
            ast_node_type::LITERAL,
            "",
            lex::literals.back()
        };
    }
}