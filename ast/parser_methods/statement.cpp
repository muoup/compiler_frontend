#include "statement.h"

#include <charconv>
#include <stdexcept>

#include "expression.h"
#include "operator.h"
#include "program.h"
#include "../util.h"
#include "../../lexer/lex.h"

using namespace ast;

std::unique_ptr<nodes::statement> pm::parse_statement(lex_cptr &ptr, lex_cptr end) {
    if (ptr == end)
        return nullptr;

    if (ptr->span == "if") {
        return std::make_unique<nodes::if_statement>(parse_if_statement(ptr, end));
    } else if (ptr->span == "while" || ptr->span == "do") {
        return std::make_unique<nodes::loop>(parse_loop(ptr, end));
    } else if (ptr->span == "for") {
        return std::make_unique<nodes::for_loop>(parse_for_loop(ptr, end));
    } else if (ptr->span == "return") {
        if (++ptr == end)
            return std::make_unique<nodes::return_op>();

        return std::make_unique<nodes::return_op>(
            parse_until(ptr, end, ";", parse_expression)
        );
    }

    return std::make_unique<nodes::expression_root>(
        parse_until(ptr, end, ";", parse_expression)
    );
}

nodes::if_statement pm::parse_if_statement(lex_cptr &ptr, lex_cptr end) {
    assert_token_val(ptr, "if");

    return nodes::if_statement {
        parse_between(ptr, "(", parse_expression),
        parse_body(ptr, end),
        test_token_val(ptr, "else") ?
            std::make_optional<nodes::scope_block>(parse_body(ptr, end)) :
            std::nullopt
    };
}

nodes::loop pm::parse_loop(lex_cptr &ptr, lex_cptr end) {
    if (test_token_val(ptr, "do")) {
        auto stmts = parse_body(ptr, end);
        assert_token_val(ptr, "while");
        auto condition = parse_between(ptr, "(", parse_expression);
        assert_token_val(ptr, ";");

        return nodes::loop {
            false,
            std::move(condition),
            std::move(stmts)
        };
    } else if (test_token_val(ptr, "while")) {
        return nodes::loop {
            true,
            parse_between(ptr, "(", parse_expression),
            parse_body(ptr, end)
        };
    } else {
        throw std::runtime_error("Expected 'do' or 'while'");
    }
}

nodes::for_loop pm::parse_for_loop(lex_cptr &ptr, lex_cptr end) {
    assert_token_val(ptr, "for");
    assert_token_val(ptr, "(");

    return nodes::for_loop {
        parse_until(ptr, end, ";", parse_expression),
        parse_until(ptr, end, ";", parse_expression),
        parse_until(ptr, end, ")", parse_expression),
        parse_body(ptr, end)
    };
}

nodes::type_instance pm::parse_type_instance(lex_cptr &ptr, lex_cptr end) {
    return nodes::type_instance {
        parse_value_type(ptr, end),
        assert_token_type(ptr, lex::lex_type::IDENTIFIER)->span,
    };
}

nodes::un_op pm::parse_unop(lex_cptr &ptr, const lex_cptr end) {
    const auto operator_type = assert_token_type(ptr, lex::lex_type::EXPR_SYMBOL)->span;
    auto expr = parse_value(++ptr, end);

    if (!expr)
        throw std::runtime_error("Expected value after operator");

    ++ptr;

    return nodes::un_op {
        unwrap_or_throw(get_unop(operator_type)),
        std::move(expr)
    };
}

nodes::bin_op pm::parse_binop(lex_cptr &ptr, const lex_cptr) {
    const auto op = assert_token_type(ptr, lex::lex_type::EXPR_SYMBOL)->span;
    const auto op_type = unwrap_or_throw(get_binop(op));

    return nodes::bin_op {
        op_type,
        nullptr,
        nullptr
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

std::unique_ptr<nodes::expression> pm::parse_value(lex_cptr &ptr, const lex_cptr end) {
    if (auto literal = parse_literal(ptr, end))
        return std::make_unique<nodes::literal>(std::move(literal.value()));

    if (ptr->span == "(")
        return parse_between(ptr, parse_expression);

    if (is_variable_identifier(ptr) && (ptr + 1)->type == lex::lex_type::IDENTIFIER)
        return std::make_unique<nodes::initialization>(parse_type_instance(ptr, end));

    if (ptr->type == lex::lex_type::IDENTIFIER && (ptr + 1)->span == "(")
        return std::make_unique<nodes::method_call>(parse_method_call(ptr, end));

    if (ptr->type == lex::lex_type::IDENTIFIER)
        return std::make_unique<nodes::var_ref>(ptr++->span);

    return nullptr;
}

std::optional<nodes::literal> pm::parse_literal(lex_cptr &ptr, lex_cptr) {
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