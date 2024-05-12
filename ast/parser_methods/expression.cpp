#include "expression.h"

#include <memory>
#include <stack>
#include <stdexcept>

#include "operator.h"
#include "statement.h"
#include "../../lexer/lex.h"
#include "../util.h"
#include "program.h"
#include "../data/data_maps.h"

using namespace ast;
using namespace ast::pm;

std::unique_ptr<nodes::expression> pm::parse_expression(lex_cptr &ptr, const lex_cptr end) {
    if (auto un_op = find_element(unop_type_map,ptr->span))
        return parse_unop(ptr, end);

    if (auto literal = parse_literal(ptr, end))
        return std::make_unique<nodes::literal>(std::move(literal.value()));

    if (peek(ptr, end)->span == "(")
        return parse_between(ptr, parse_expr_tree);

    if (peek(ptr, end)->span == "{")
        return std::make_unique<nodes::initializer_list>(parse_initializer_list(ptr, end));

    if (peek(ptr, end)->span == "match")
        return std::make_unique<nodes::match>(parse_match(ptr, end));

    if (is_variable_identifier(ptr))
        return std::make_unique<nodes::initialization>(parse_type_instance(ptr, end));

    if (try_peek_type(ptr, end, lex::lex_type::IDENTIFIER) && try_peek_val(ptr, end, "(", 1))
        return std::make_unique<nodes::method_call>(parse_method_call(ptr, end));

    if (try_peek_type(ptr, end, lex::lex_type::IDENTIFIER) && try_peek_val(ptr, end, "[", 1))
        return std::make_unique<nodes::bin_op>(parse_array_access(ptr, end));

    if (try_peek_type(ptr, end, lex::lex_type::IDENTIFIER))
        return std::make_unique<nodes::var_ref>(parse_variable(ptr, end));

    return nullptr;
}

std::unique_ptr<nodes::expression> pm::parse_expr_tree(lex_cptr &ptr, const lex_cptr end) {
    std::stack<std::unique_ptr<nodes::expression>> expr_stack;
    std::stack<nodes::bin_op_type> binop_stack;

    const auto combine_back = [&expr_stack, &binop_stack] {
        nodes::bin_op_type top_op = binop_stack.top();
        binop_stack.pop();
        std::unique_ptr<nodes::expression> r_expr = std::move(expr_stack.top());
        expr_stack.pop();
        std::unique_ptr<nodes::expression> l_expr = std::move(expr_stack.top());
        expr_stack.pop();

        expr_stack.emplace(
            std::make_unique<nodes::bin_op>(
                    create_bin_op(std::move(l_expr), std::move(r_expr), top_op)
            )
        );
    };

    expr_stack.emplace(parse_expression(ptr, end));

    while (ptr < end) {
        if (peek(ptr, end)->type == lex::lex_type::ASSN_SYMBOL) {
            auto assn_type = parse_assn(end, ptr);
            auto lhs = std::move(expr_stack.top());
            auto rhs = parse_expr_tree(ptr, end);
            rhs = load_if_necessary(std::move(rhs));

            expr_stack.pop();

            return std::make_unique<nodes::assignment>(
                    create_assignment(std::move(lhs), std::move(rhs), assn_type)
            );
        }

        auto bin_op = parse_binop(ptr, end);

        if (bin_op == std::nullopt)
            break;

        while (!binop_stack.empty() && *find_element(binop_prec, *bin_op) <= *find_element(binop_prec, binop_stack.top()))
            combine_back();

        expr_stack.emplace(parse_expression(ptr, end));
        binop_stack.emplace(*bin_op);
    }

    while (!binop_stack.empty())
        combine_back();

    return std::move(expr_stack.top());
}

std::unique_ptr<nodes::expression> pm::parse_unop(lex_cptr &ptr, const lex_cptr end) {
    const auto operator_type = assert_token_type(ptr, lex::lex_type::EXPR_SYMBOL)->span;
    auto expr = parse_expression(ptr, end);

    if (!expr)
        throw std::runtime_error("Expected value after operator");

    auto unop = *find_element(unop_type_map, operator_type);

    switch (unop) {
        case nodes::un_op_type::addr_of:
            return std::make_unique<nodes::expression_shield>(
                std::move(expr)
            );
        case nodes::un_op_type::deref:
            return std::make_unique<nodes::load>(
                std::move(expr)
            );
        default:
            return std::make_unique<nodes::un_op>(
                unop,
                std::move(expr)
            );
    }
}

std::optional<nodes::bin_op_type> pm::parse_binop(lex_cptr &ptr, const lex_cptr) {
    auto op = test_token_type(ptr, lex::lex_type::EXPR_SYMBOL);

    if (!op)
        return std::nullopt;

    return find_element(binop_type_map,(*op)->span);
}

std::optional<nodes::bin_op_type> pm::parse_assn(const ast::lex_cptr, ast::lex_cptr &ptr) {
    auto type = assert_token_type(ptr, lex::lex_type::ASSN_SYMBOL)->span;

    if (type.size() == 1)
        return std::nullopt;

    auto op = type.substr(0, 1);
    auto bin_op = find_element(binop_type_map, op);

    if (!bin_op)
        throw std::runtime_error("Invalid assignment operator");

    return bin_op;
}

std::optional<nodes::literal> pm::parse_literal(lex_cptr &ptr, const lex_cptr end) {
    int i = 0;
    double d = 0;

    switch (peek(ptr, end)->type) {
        case lex::lex_type::INT_LITERAL:
            std::from_chars(peek(ptr, end)->span.data(), peek(ptr, end)->span.data() + peek(ptr, end)->span.size(), i);
            consume(ptr, end);
            return nodes::literal { i };
        case lex::lex_type::FLOAT_LITERAL:
            std::from_chars(peek(ptr, end)->span.data(), peek(ptr, end)->span.data() + peek(ptr, end)->span.size(), d);
            consume(ptr, end);
            return nodes::literal { d };
        case lex::lex_type::STRING_LITERAL:
            return nodes::literal { consume(ptr, end)->span };
        case lex::lex_type::CHAR_LITERAL:
            return nodes::literal { consume(ptr, end)->span.front() };
        default:
            return std::nullopt;
    }
}

nodes::type_instance pm::parse_type_instance(lex_cptr &ptr, const lex_cptr end) {
    auto val_type = pm::parse_var_type(ptr, end);
    auto type = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;

    scope_stack.back().emplace(type, val_type);

    return nodes::type_instance {
            val_type,
            type
    };
}

nodes::method_call pm::parse_method_call(lex_cptr &ptr, const lex_cptr end) {
    const auto method_name = consume(ptr, end)->span;
    const auto param_end = consume(ptr, end)->closer.value();

    nodes::method_call method_call {
            method_name,
            parse_call_params(ptr, param_end)
    };

    ptr = param_end + 1;

    return method_call;
}

nodes::bin_op pm::parse_array_access(lex_cptr &ptr, const lex_cptr end) {
    const auto var_name = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;
    auto array_index = load_if_necessary(
            parse_between(ptr, "[", parse_expr_tree)
    );

    return nodes::bin_op{
            nodes::bin_op_type::acc,
            std::make_unique<nodes::var_ref>(
                    var_name,
                    get_var_type(var_name)
            ),
            std::move(array_index)
    };

}

nodes::var_ref pm::parse_variable(ast::lex_cptr &ptr, const ast::lex_cptr end) {
    auto name = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;
    auto type = get_var_type(name);

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

        match.cases.emplace_back(nodes::match_case {
            std::move(match_expr),
            std::move(body)
        });
        if (!test_token_val(ptr, ","))
            break;
    }

    assert_token_val(ptr, "}");

    return match;
}

nodes::initializer_list pm::parse_initializer_list(ast::lex_cptr &ptr, const ast::lex_cptr end) {
    return nodes::initializer_list {
        parse_between(ptr, parse_call_params)
    };
}

nodes::struct_initializer pm::parse_struct_initializer(ast::lex_cptr &ptr, const ast::lex_cptr end) {
    auto struct_type = assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span;

    if (!struct_types.contains(struct_type))
        throw std::runtime_error(std::format("Struct {} not found", struct_type));

    return nodes::struct_initializer {
        struct_type,
        parse_initializer_list(ptr, end).values
    };
}