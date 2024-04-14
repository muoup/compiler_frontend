#include "expression.h"

#include <memory>
#include <stack>
#include <stdexcept>

#include "operator.h"
#include "statement.h"
#include "../../lexer/lex.h"
#include "../util.h"

using namespace ast;

std::unique_ptr<nodes::expression> pm::parse_expression(lex_cptr &ptr, const lex_cptr end) {
    std::stack<std::unique_ptr<nodes::expression>> expr_stack;
    std::stack<std::unique_ptr<nodes::bin_op>> binop_stack;

    const auto gen_expr = [&ptr, end] -> std::unique_ptr<nodes::expression> {
        auto val = parse_value(ptr, end);

        if (val)
            return val;

        if (ptr >= end - 1)
            throw std::runtime_error("Expected value");

        return std::make_unique<nodes::un_op>(parse_unop(ptr, end));
    };

    const auto combine_back = [&expr_stack, &binop_stack] {
        auto& top_op = binop_stack.top();

        top_op->right = std::move(expr_stack.top());
        expr_stack.pop();

        top_op->left = std::move(expr_stack.top());
        expr_stack.pop();

        expr_stack.emplace(std::move(top_op));
        binop_stack.pop();
    };

    expr_stack.emplace(gen_expr());

    while (ptr < end) {
        if (const auto tok = test_token_type(ptr, lex::lex_type::ASSN_SYMBOL)) {
            while (!binop_stack.empty())
                combine_back();

            auto lhs = std::move(expr_stack.top());
            expr_stack.pop();

            auto rhs = parse_expression(ptr, end);
            const auto assn_str = tok.value()->span;

            if (assn_str.size() == 1) {
                return std::make_unique<nodes::assignment>(std::move(lhs), std::move(rhs));
            }

            auto binop = get_binop(std::string_view { assn_str.begin(), assn_str.end() - 1 });

            if (!binop)
                throw std::runtime_error("Invalid assignment operator");

            return std::make_unique<nodes::assignment>(
                std::move(lhs),
                std::move(rhs),
                binop.value()
            );
        }

        auto r_op = std::make_unique<nodes::bin_op>(parse_binop(ptr, end));

        while (!binop_stack.empty() && get_prec(*r_op) <= get_prec(*binop_stack.top()))
            combine_back();

        expr_stack.emplace(gen_expr());
        binop_stack.emplace(std::move(r_op));
    }

    while (!binop_stack.empty())
        combine_back();

    return std::move(expr_stack.top());
}