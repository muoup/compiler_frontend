#include "abstract_data.h"

#include "data_maps.h"

using namespace ast::nodes;

variable_type variable_type::pointer_to() const {
    auto temp = *this;
    temp.pointer_depth++;
    return temp;
}

variable_type variable_type::dereference() const {
    auto temp = *this;

    if (temp.pointer_depth == 0)
        throw std::runtime_error("Cannot dereference non-pointer type!");

    temp.pointer_depth--;
    return temp;
}

variable_type variable_type::change_var_ref(bool new_val) const {
    auto temp = *this;
    temp.is_var_ref = new_val;
    return temp;
}

bool variable_type::is_intrinsic() const {
    return std::holds_alternative<intrinsic_types>(type);
}

bool variable_type::is_pointer() const {
    return pointer_depth > 0;
}

bool variable_type::is_fp() const {
    if (!is_intrinsic())
        return false;

    auto lit_type = std::get<intrinsic_types>(type);

    return lit_type == intrinsic_types::f32 || lit_type == intrinsic_types::f64;
}

bool variable_type::operator ==(const variable_type &other) const {
    return type == other.type && pointer_depth == other.pointer_depth;
}

std::string variable_type::type_str() const {
    std::string_view type_str = is_intrinsic() ?
           *ast::pm::find_key(ast::pm::intrin_map, std::get<intrinsic_types>(type))
              : std::get<std::string_view>(type);

    return std::string(type_str) + std::string(pointer_depth, '*');
}