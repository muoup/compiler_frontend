#include "abstract_data.h"

#include <unordered_map>

using namespace ast::nodes;

const std::unordered_map<std::string_view, intrinsic_types> intrin_map {
    { "i8", intrinsic_types::i8 },
    { "i16", intrinsic_types::i16 },
    { "i32", intrinsic_types::i32 },
    { "i64", intrinsic_types::i64 },

    { "u8", intrinsic_types::u8 },
    { "u16", intrinsic_types::u16 },
    { "u32", intrinsic_types::u32 },
    { "u64", intrinsic_types::u64 },

    { "f32", intrinsic_types::f32 },
    { "f64", intrinsic_types::f64 },

    { "char", intrinsic_types::char_ },
    { "bool", intrinsic_types::bool_ },
    { "void", intrinsic_types::void_ }
};

std::optional<intrinsic_types> ast::nodes::get_intrinsic_type(std::string_view type) {
    const auto it = intrin_map.find(type);

    return it != intrin_map.end() ?
        std::make_optional(it->second) :
        std::nullopt;
}
