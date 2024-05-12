#include "ast_nodes.h"
#include "../parser_methods/operator.h"
#include "data_maps.h"
#include "../util.h"
#include <iostream>

using namespace ast::nodes;

// -- Specialized Constructors --

array_initializer::array_initializer(variable_type type, ast::nodes::initializer_list &&init_list)
    : array_type(type), values(std::move(init_list.values)) {

    for (auto &val : values) {
        const auto val_type = val->get_type();

        if (val_type != array_type) {
            if (!val_type.is_intrinsic() || !array_type.is_intrinsic())
                throw std::runtime_error("Non-intrinsic types cannot be casted (yet)!");

            val = std::make_unique<cast>(std::move(val), array_type);
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