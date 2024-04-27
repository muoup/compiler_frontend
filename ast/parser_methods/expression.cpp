#include "expression.h"

#include <memory>
#include <stack>
#include <stdexcept>

#include "operator.h"
#include "statement.h"
#include "../../lexer/lex.h"
#include "../util.h"
#include "program.h"

using namespace ast;
using namespace ast::pm;

std::unique_ptr<nodes::expression> pm::parse_expression(lex_cptr &ptr, const lex_cptr end) {
    if (auto un_op = get_unop(ptr->span))
        return std::make_unique<nodes::un_op>(parse_unop(ptr, end));

    if (auto literal = parse_literal(ptr, end))
        return std::make_unique<nodes::literal>(std::move(literal.value()));

    if (peek(ptr, end)->span == "(")
        return parse_between(ptr, parse_expr_tree);

    if (peek(ptr, end)->span == "{")
        return std::make_unique<nodes::scope_block>(parse_body(ptr, end));

    if (peek(ptr, end)->span == "match")
        return std::make_unique<nodes::match>(parse_match(ptr, end));

    if (is_variable_identifier(ptr) && peek(ptr, end, 1)->type == lex::lex_type::IDENTIFIER)
        return std::make_unique<nodes::initialization>(parse_type_instance(ptr, end));

    if (peek(ptr, end)->type == lex::lex_type::IDENTIFIER && peek(ptr, end, 1)->span == "(")
        return std::make_unique<nodes::method_call>(parse_method_call(ptr, end));

    if (peek(ptr, end)->type == lex::lex_type::IDENTIFIER)
        return std::make_unique<nodes::var_ref>(parse_variable(ptr, end));

    return nullptr;
}

std::unique_ptr<nodes::expression> pm::parse_expr_tree(lex_cptr &ptr, const lex_cptr end) {
    std::stack<std::unique_ptr<nodes::expression>> expr_stack;
    std::stack<std::unique_ptr<nodes::bin_op>> binop_stack;

    const auto combine_back = [&expr_stack, &binop_stack] {
        auto& top_op = binop_stack.top();

        top_op->right = std::move(expr_stack.top());
        expr_stack.pop();

        top_op->left = std::move(expr_stack.top());
        expr_stack.pop();

        expr_stack.emplace(std::move(top_op));
        binop_stack.pop();
    };

    expr_stack.emplace(parse_expression(ptr, end));

    while (ptr < end) {
        if (const auto tok = test_token_type(ptr, lex::lex_type::ASSN_SYMBOL)) {
            while (!binop_stack.empty())
                combine_back();

            auto lhs = std::move(expr_stack.top());
            expr_stack.pop();

            auto rhs = parse_expr_tree(ptr, end);
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

        auto binop = parse_binop(ptr, end);

        if (binop == std::nullopt)
            break;

        auto r_op = std::make_unique<nodes::bin_op>(std::move(binop.value()));

        while (!binop_stack.empty() && get_prec(*r_op) <= get_prec(*binop_stack.top()))
            combine_back();

        expr_stack.emplace(parse_expression(ptr, end));
        binop_stack.emplace(std::move(r_op));
    }

    while (!binop_stack.empty())
        combine_back();

    return std::move(expr_stack.top());
}

nodes::un_op pm::parse_unop(lex_cptr &ptr, const lex_cptr end) {
    const auto operator_type = assert_token_type(ptr, lex::lex_type::EXPR_SYMBOL)->span;
    auto expr = parse_expression(++ptr, end);

    if (!expr)
        throw std::runtime_error("Expected value after operator");

    ++ptr;

    return nodes::un_op {
            unwrap_or_throw(get_unop(operator_type)),
            std::move(expr)
    };
}

std::optional<nodes::bin_op> pm::parse_binop(lex_cptr &ptr, const lex_cptr) {
    auto op = test_token_type(ptr, lex::lex_type::EXPR_SYMBOL);

    if (!op)
        return std::nullopt;

    auto op_type = get_binop(op.value()->span);

    if (!op_type)
        return std::nullopt;

    return nodes::bin_op {
            op_type.value(),
            nullptr,
            nullptr
    };
}

std::optional<nodes::literal> pm::parse_literal(lex_cptr &ptr, const lex_cptr) {
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

nodes::type_instance pm::parse_type_instance(lex_cptr &ptr, const lex_cptr end) {
    auto val_type = pm::parse_value_type(ptr, end);
    auto type = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;

    scope_stack.back().emplace(type, val_type);

    return nodes::type_instance {
            val_type,
            type
    };
}

nodes::method_call pm::parse_method_call(lex_cptr &ptr, const lex_cptr) {
    const auto method_name = ptr++->span;
    const auto param_end = ptr++->closer.value();

    nodes::method_call method_call {
            method_name,
            parse_call_params(ptr, param_end)
//        parse_split<std::unique_ptr<nodes::expression>, lex_cptr>(ptr, param_end, ",", parse_expression)
    };

    ptr = param_end + 1;

    return method_call;
}

nodes::var_ref pm::parse_variable(ast::lex_cptr &ptr, const ast::lex_cptr end) {
    auto name = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;
    auto type = get_variable_type(name);

    return nodes::var_ref {
            name,
            type
    };
}

nodes::match pm::parse_match(ast::lex_cptr &ptr, const ast::lex_cptr end) {
    nodes::match match;

    assert_token_val(ptr, "match");
    match.match_expr = parse_expression(ptr, end);

    assert_token_val(ptr, "{");

    while (peek(ptr, end)->span != "}") {
        if (test_token_val(ptr, "default")) {
            match.default_case = std::make_unique<nodes::scope_block>(parse_body(ptr, end));
            break;
        }

        assert_token_val(ptr, "case");

        auto match_expr = parse_expr_tree(ptr, end);
        auto body = parse_body(ptr, end);

        match.cases.emplace_back(nodes::match::match_case {
            std::move(match_expr),
            std::move(body)
        });
        if (!test_token_val(ptr, ","))
            break;
    }

    assert_token_val(ptr, "}");

    return match;
}