#include "ast_nodes.h"
#include "../parser_methods/operator.h"
#include "data_maps.h"
#include "../util.h"
#include <iostream>

using namespace ast::nodes;

// -- Specialized Constructors --

array_initializer::array_initializer(variable_type type, ast::nodes::initializer_list &&init_list)
    : array_type(type), values(std::move(init_list.values)) {

    const auto expected_type = array_type.dereference();

    for (auto &val : values) {
        const auto val_type = val->get_type();

        if (val_type != expected_type) {
            if (!val_type.is_intrinsic() || !array_type.is_intrinsic())
                throw std::runtime_error("Non-intrinsic types cannot be casted (yet)!");

            val = std::make_unique<cast>(std::move(val), expected_type);
        }
    }
}

struct_initializer::struct_initializer(const struct_declaration* type, std::vector<std::unique_ptr<nodes::expression>> init_list)
    : struct_type(type->struct_name), values(std::move(init_list)) {
    const auto &fields = type->fields;

    if (fields.size() != values.size())
        throw std::runtime_error("Initializers do not match struct fields!");

    for (int i = 0; i < values.size(); ++i) {
        const auto val_type = values[i]->get_type();
        const auto expected_type = fields[i].type;

        if (val_type != expected_type) {
            if (!val_type.is_intrinsic() || !expected_type.is_intrinsic())
                throw std::runtime_error("Non-intrinsic types cannot be casted (yet)!");

            values[i] = std::make_unique<cast>(std::move(values[i]), expected_type);
        }
    }
}