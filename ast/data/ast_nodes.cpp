#include "ast_nodes.h"
#include <iostream>

#include "../parser_methods/operator.h"

using namespace ast::nodes;

void print_depth(const size_t depth) {
    for (size_t i = 0; i < depth; ++i)
        std::cout << "  ";
}

void root::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Root\n";
    for (const auto& var : global_vars)
        var.print(depth + 1);

    for (const auto& func : functions)
        func.print(depth + 1);
}

void code_block::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Code block\n";
    for (const auto& stmt : statements)
        stmt.print(depth + 1);
}

void function::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Function " << function_name << "\n";
    for (const auto& param : param_types)
        param.print(depth + 1);

    body.print(depth + 1);
}

void expression::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Expression\n";
    switch (value.index()) {
        case METHOD_CALL:
            std::get<method_call>(value).print(depth + 1);
            break;
        case ASSIGNMENT:
            std::get<assignment>(value).print(depth + 1);
            break;
        case CONST_ASSIGNMENT:
            std::get<const_assignment>(value).print(depth + 1);
            break;
        case VARIABLE:
            std::get<variable>(value).print(depth + 1);
            break;
        case INITIALIZATION:
            std::get<initialization>(value).print(depth + 1);
            break;
        case UN_OP:
            std::get<un_op>(value).print(depth + 1);
            break;
        case BIN_OP:
            std::get<bin_op>(value).print(depth + 1);
            break;
        case LITERAL:
            std::get<literal>(value).print(depth + 1);
            break;
        default:
            throw std::runtime_error("Invalid expression type");
    }
}

void statement::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Statement\n";

    switch (value.index()) {
        case 0:
            std::get<return_op>(value).print(depth + 1);
            break;
        case 1:
            std::get<expression>(value).print(depth + 1);
            break;
        case 2:
            std::get<conditional>(value).print(depth + 1);
            break;
        default:
            std::unreachable();
    }
}

void return_op::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Return\n";

    if (value)
        value->print(depth + 1);
}

void variable::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Variable " << name << "\n";
}

void initialization::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Initialization\n";
    var.print(depth + 1);
}

void const_assignment::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Const Assignment\n";
    variable.print(depth + 1);
    value->print(depth + 1);
}

void conditional::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Conditional " << static_cast<int>(type) << "\n";
    condition.print(depth + 1);
    body.print(depth + 1);
}

void method_call::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Method call " << method_name << "\n";
    for (const auto& arg : arguments)
        arg.print(depth + 1);
}

void bin_op::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Binary " << pm::from_binop(type) << "\n";
    left->print(depth + 1);
    right->print(depth + 1);
}

void un_op::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Unary " << pm::from_unop(type) << "\n";
    value->print(depth + 1);
}

void literal::print(const size_t depth) const {
    print_depth(depth);

    switch (value.index()) {
        case 0:
            std::cout << "[UINT] Literal " << std::get<unsigned int>(value) << "\n";
            break;
        case 1:
            std::cout << "[INT] Literal " << std::get<int>(value) << "\n";
            break;
        case 2:
            std::cout << "[DOUBLE] Literal " << std::get<double>(value) << "\n";
            break;
        case 3:
            std::cout << "[CHAR] Literal " << std::get<char>(value) << "\n";
            break;
        case 4:
            std::cout << "[STRING] Literal " << std::get<std::string_view>(value) << "\n";
            break;
        default:
            throw std::runtime_error("Invalid literal type");
    }
}

void assignment::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Assignment\n";
    if (std::holds_alternative<initialization>(variable))
        std::get<initialization>(variable).print(depth + 1);
    else
        std::cout << "Variable " << std::get<nodes::variable>(variable).name << "\n";

    value->print(depth + 1);
}