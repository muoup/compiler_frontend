#include "statement.h"

#include <stack>
#include <stdexcept>

#include "operator.h"
#include "../declarations.h"
#include "../../lexer/lex.h"

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

ast_node& add_operator(const lex_cptr ptr, ast_node& curr_root, std::stack<lex_cptr> &ident_stack, std::stack<lex_cptr> &op_stack) {
    if (pm::binop_prec.at(op_stack.top()->span) >= pm::binop_prec.at(ptr->span)) {
        op_stack.push(ptr);
        return curr_root;
    }

    auto &binop = curr_root.add_child(ast_node {
        ast_node_type::OPERATOR
    });

    auto &lroot = binop;

    while (!op_stack.empty()) {
        const auto op = lroot.add_child(ast_node_type::OPERATOR, op_stack.top()->span);
        op_stack.pop();

        auto ident = pm::parse_value(ident_stack.top());
        ident_stack.pop();

        lroot = op;
    }

    lroot.add_child(pm::parse_value(ident_stack.top()));
    ident_stack.pop();

    return binop;
}

ast_node pm::parse_expression(lex_cptr &ptr, const lex_cptr end) {
    std::stack<lex_cptr> ident_stack, op_stack;

    ast_node expr {
        ast_node_type::EXPRESSION
    };
    ast_node& root = expr;

    while (ptr != end) {
        if (binop_prec.contains(ptr->span))
            root = add_operator(ptr++, root, ident_stack, op_stack);
        else
            ident_stack.push(ptr++);
    }

    root.add_child(pm::parse_value(ident_stack.top()));
    ident_stack.pop();

    return expr;
}

ast_node pm::parse_value(const lex_cptr ptr) {
    if (ptr->type == lex::lex_type::IDENTIFIER) {
        return ast_node {
            ast_node_type::VARIABLE,
            "",
            ptr->span
        };
    } else if (lex::LITERAL_SET.contains(ptr->type)) {
        return ast_node {
            ast_node_type::LITERAL,
            "",
            ptr->span
        };
    }

    throw std::runtime_error("Invalid value");
}
