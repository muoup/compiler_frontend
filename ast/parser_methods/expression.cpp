#include "expression.h"

#include <memory>
#include <stack>
#include <stdexcept>

#include "operator.h"
#include "statement.h"
#include "../data/ast_nodes.h"
#include "../../lexer/lex.h"

using namespace ast;

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
        auto right = std::move(expr_stack.top());
        expr_stack.pop();
        auto left = std::move(expr_stack.top());
        expr_stack.pop();
        auto binop_type = binop_stack.top().type;
        binop_stack.pop();

        switch (left.value.index()) {
            case nodes::expression_type::VAR_REF:
                expr_stack.emplace(
                    pure_assignment(
                        left,
                        right,
                        binop_type
                    )
                );
                break;
            case nodes::expression_type::TYPE_INSTANCE:
                expr_stack.emplace(
                    assign_initialization(
                        left,
                        right,
                        binop_type
                    )
                );
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

nodes::expression pm::assign_initialization(const nodes::expression &lhs, nodes::expression &rhs, const nodes::bin_op_type type) {
    if (type != nodes::bin_op_type::invalid)
        throw std::runtime_error("Initialization can only be a pure assignment");

    return nodes::expression {
        nodes::initialization {
            std::get<nodes::type_instance>(lhs.value),
            std::make_unique<nodes::expression>(std::move(rhs)),
        }
    };
}

nodes::expression pm::pure_assignment(nodes::expression &lhs, nodes::expression &rhs, nodes::bin_op_type op) {
    if (std::holds_alternative<nodes::un_op>(lhs.value)) {
        const auto unop = std::get<nodes::un_op>(std::move(lhs.value));

        if (unop.type != nodes::un_op_type::deref
        ||  std::holds_alternative<nodes::var_ref>(unop.value->value))
            throw std::runtime_error("Invalid assignment target");

        return nodes::expression {
            nodes::var_modification {
                true,
                std::get<nodes::var_ref>(unop.value->value).name,
                std::make_unique<nodes::expression>(std::move(rhs)),
                op
            }
        };
    }

    if (std::holds_alternative<nodes::var_ref>(lhs.value)) {
        return nodes::expression {
            nodes::var_modification {
                false,
                std::get<nodes::type_instance>(lhs.value).var_name,
                std::make_unique<nodes::expression>(std::move(rhs)),
            }
        };
    }

    throw std::runtime_error("Invalid assignment target");
}