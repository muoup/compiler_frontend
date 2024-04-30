#include "ast_nodes.h"
#include "../parser_methods/operator.h"
#include "data_maps.h"
#include "../util.h"
#include <iostream>

using namespace ast::nodes;

void print_depth(const size_t depth) {
    for (size_t i = 0; i < depth; ++i)
        std::cout << "  ";
}

void root::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Root\n";
    for (const auto& stmt : program_level_statements)
        stmt->print(depth + 1);
}

void scope_block::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Code block\n";
    for (const auto& stmt : statements)
        stmt->print(depth + 1);
}

void function::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Function " << fn_name << "\n";

    print_depth(depth + 1);
    std::cout << "Return Type\n";
    return_type.print(depth + 2);

    if (!param_types.empty()) {
        print_depth(depth);
        std::cout << "Parameters\n";
    }

    for (const auto& param : param_types)
        param.print(depth + 1);

    body.print(depth + 1);
}

void return_op::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Return\n";

    if (val)
        val->print(depth + 1);
}

void var_ref::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Variable Reference: " << name << "\n";

    if (type)
        type->print(depth + 1);
    else {
        print_depth(depth + 1);
        std::cout << "Type Not Found\n";
    }
}

void type_instance::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Initialization: " << var_name << '\n';
    type.print(depth + 1);
}

void value_type::print(size_t depth) const {
    print_depth(depth);

    std::cout << "Type: ";

    if (!std::holds_alternative<intrinsic_types>(type)) {
        std::cout << std::get<std::string_view>(type);
    } else {
        auto intrinsic = std::get<intrinsic_types>(type);

        for (const auto &[key, val]: pm::intrin_map) {
            if (val == intrinsic) {
                std::cout << "" << key;
                break;
            }
        }
    }

    for (int _ = 0; _ < pointer_depth; ++_)
        std::cout << "*";

    std::cout << '\n';
}

void initialization::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Variable Initialization\n";
    variable.print(depth + 1);
}

void loop::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Loop; pre-eval=" << (pre_eval ? "true" : "false") << "\n";
    condition->print(depth + 1);
    body.print(depth + 1);
}

void if_statement::print(const size_t depth) const {
    print_depth(depth);
    std::cout << "If Statement\n";
    condition->print(depth + 1);
    body.print(depth + 1);

    if (!else_body)
        return;

    print_depth(depth);
    std::cout << "Else\n";

    else_body->print(depth + 1);
}

void method_call::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Method call " << method_name << "\n";
    for (const auto& arg : arguments)
        arg->print(depth + 1);
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

void cast::print(size_t depth) const {
    print_depth(depth);

    std::cout << "Implicit Cast\n";
    cast_type.print(depth + 1);
    expr->print(depth + 1);
}

void load::print(size_t depth) const {
    print_depth(depth);

    std::cout << "Implicit Dereference\n";
    expr->print(depth + 1);
}

void initializer_list::print(size_t depth) const {
    print_depth(depth);

    std::cout << "Initializer List\n";
    for (const auto& val : values)
        val->print(depth + 1);
}

void array_initializer::print(size_t depth) const {
    print_depth(depth);

    std::cout << "Array List\n";
    for (const auto& val : values)
        val->print(depth + 1);
}

void struct_initializer::print(size_t depth) const {
    print_depth(depth);

    std::cout << "Struct Initializer " << struct_type << "\n";

    for (const auto& val : values)
        val->print(depth + 1);
}

void literal::print(const size_t depth) const {
    print_depth(depth);

    switch (value.index()) {
        case 0:
            std::cout << "Unsigned Int Literal " << std::get<unsigned int>(value) << "\n";
            break;
        case 1:
            std::cout << "Int Literal " << std::get<int>(value) << "\n";
            break;
        case 2:
            std::cout << "Double Literal " << std::get<double>(value) << "\n";
            break;
        case 3:
            std::cout << "Char Literal " << std::get<char>(value) << "\n";
            break;
        case 4:
            std::cout << "String Literal " << std::get<std::string_view>(value) << "\n";
            break;
        default:
            throw std::runtime_error("Invalid literal type");
    }
}

void assignment::print(const size_t depth) const {
    print_depth(depth);

    std::cout << "Assignment";

    if (op)
        std::cout << " " << pm::from_binop(*op);

    std::cout << '\n';

    lhs->print(depth + 1);
    rhs->print(depth + 1);
}

void for_loop::print(size_t depth) const {
    print_depth(depth);
    std::cout << "For loop\n";

    print_depth(depth + 1);
    std::cout << "Init\n";
    init->print(depth + 2);

    print_depth(depth + 1);
    std::cout << "Condition\n";
    condition->print(depth + 2);

    print_depth(depth + 1);
    std::cout << "Update\n";
    update->print(depth + 2);

    print_depth(depth + 1);
    std::cout << "Body\n";
    body.print(depth + 2);
}

void struct_declaration::print(size_t depth) const {
    print_depth(depth);
    std::cout << "Struct " << name << "\n";

    for (const auto& field : fields)
        field.print(depth + 1);
}

void match::print(size_t depth) const {
    print_depth(depth);
    std::cout << "Match\n";

    print_depth(depth + 1);
    std::cout << "Matching:\n";
    match_expr->print(depth + 2);

    for (const auto& case_ : cases) {
        print_depth(depth + 1);
        std::cout << "Case\n";

        print_depth(depth + 2);
        std::cout << "Pattern\n";
        case_.match_expr->print(depth + 3);

        print_depth(depth + 2);
        std::cout << "Body\n";
        case_.body.print(depth + 3);
    }

    if (default_case) {
        print_depth(depth + 1);
        std::cout << "Default\n";
        default_case->print(depth + 2);
    }
}

// -- Specialized Constructors --

array_initializer::array_initializer(value_type type, ast::nodes::initializer_list &&init_list)
    : array_type(type), values(std::move(init_list.values)) {

    for (auto &val : values) {
        const auto val_type = val->get_type();
        const auto expected_type = array_type.pointer_to();

        if (val_type != expected_type) {
            if (!val_type.is_intrinsic() || !expected_type.is_intrinsic())
                throw std::runtime_error("Non-intrinsic types cannot be casted (yet)!");

            val = std::make_unique<cast>(std::move(val), expected_type);
        }
    }
}

struct_initializer::struct_initializer(std::string_view struct_type, std::vector<std::unique_ptr<nodes::expression>> init_list)
    : struct_type(struct_type), values(std::move(init_list)) {
    auto find = ast::struct_types.find(struct_type);

    if (find == ast::struct_types.end())
        throw std::runtime_error("Struct type not found");

    const auto &struct_decl = find->second;

    if (struct_decl.size() != values.size())
        throw std::runtime_error("Initializers do not match struct fields!");

    for (int i = 0; i < values.size(); ++i) {
        const auto val_type = values[i]->get_type();
        const auto expected_type = struct_decl[i].type;

        if (val_type != expected_type) {
            if (!val_type.is_intrinsic() || !expected_type.is_intrinsic())
                throw std::runtime_error("Non-intrinsic types cannot be casted (yet)!");

            values[i] = std::make_unique<cast>(std::move(values[i]), expected_type);
        }
    }
}