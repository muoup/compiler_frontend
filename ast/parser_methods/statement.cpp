#include "statement.h"

#include <charconv>
#include <stdexcept>

#include "expression.h"
#include "operator.h"
#include "program.h"
#include "../../lexer/lex.h"

using namespace ast;

std::unique_ptr<nodes::statement> pm::parse_statement(lex_cptr &ptr, lex_cptr end) {
    if (ptr == end)
        return nullptr;

    auto next = peek(ptr, end)->span;

    if (next == "if") {
        return std::make_unique<nodes::if_statement>(parse_if_statement(ptr, end));
    } else if (next == "while" || next == "do") {
        return std::make_unique<nodes::loop>(parse_loop(ptr, end));
    } else if (next == "for") {
        return std::make_unique<nodes::for_loop>(parse_for_loop(ptr, end));
    } else if (next == "return") {
        if (++ptr == end)
            return std::make_unique<nodes::return_op>();

        auto ret_val = load_if_necessary(parse_expr_tree(ptr, end));

        if (ret_val->get_type() != current_function->return_type) {
            // TODO: Attempt cast method, to prevent bad casting
            ret_val = std::make_unique<nodes::cast>(
                std::move(ret_val),
                nodes::variable_type {
                    current_function->return_type
                }
            );
        }

        return std::make_unique<nodes::return_op>(std::move(ret_val));
    }

    return std::make_unique<nodes::expression_root>(
        parse_expr_tree(ptr, end)
    );
}

nodes::if_statement pm::parse_if_statement(lex_cptr &ptr, lex_cptr end) {
    assert_token_val(ptr, "if");

    return nodes::if_statement {
        parse_between(ptr, "(", parse_expr_tree),
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
        auto condition = parse_between(ptr, "(", parse_expr_tree);
        assert_token_val(ptr, ";");

        return nodes::loop {
            false,
            std::move(condition),
            std::move(stmts)
        };
    } else if (test_token_val(ptr, "while")) {
        return nodes::loop {
            true,
            parse_between(ptr, "(", parse_expr_tree),
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
        parse_until(ptr, end, ";", parse_expr_tree),
        parse_until(ptr, end, ";", parse_expr_tree),
        parse_until(ptr, end, ")", parse_expr_tree),
        parse_body(ptr, end)
    };
}