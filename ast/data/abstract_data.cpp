#include "abstract_data.h"
#include <unordered_set>
#include <algorithm>

#include "data_maps.h"
#include "../util.h"

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
    return std::holds_alternative<intrinsic_type>(type);
}

bool variable_type::is_pointer() const {
    return pointer_depth > 0;
}

bool variable_type::is_int() const {
    return is_intrinsic() && !is_fp();
}

bool variable_type::is_signed() const {
    const static std::unordered_set<intrinsic_type> signed_types = {
            intrinsic_type::i8, intrinsic_type::i16, intrinsic_type::i32, intrinsic_type::i64
    };

    if (!is_intrinsic())
        return false;

    auto lit_type = std::get<intrinsic_type>(type);

    return signed_types.contains(lit_type);
}

bool variable_type::is_fp() const {
    if (!is_intrinsic())
        return false;

    auto lit_type = std::get<intrinsic_type>(type);

    return lit_type == intrinsic_type::f32 || lit_type == intrinsic_type::f64;
}

size_t variable_type::get_size() const {
    if (is_intrinsic())
        return *pm::find_element(intrinsic_size, std::get<intrinsic_type>(type));

    auto struct_type = *pm::find_element(struct_types, std::get<std::string_view>(type));
    size_t size = 0;

    for (const auto &field : struct_type)
        size += field.type.get_size();

    return size;
}

bool variable_type::operator ==(const variable_type &other) const {
    return type == other.type && pointer_depth == other.pointer_depth;
}

std::string variable_type::type_str() const {
    std::string_view type_str = is_intrinsic() ?
           *ast::pm::find_key(ast::pm::intrin_map, std::get<intrinsic_type>(type))
              : std::get<std::string_view>(type);

    return std::string(type_str) + std::string(pointer_depth, '*');
}