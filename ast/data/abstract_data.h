#pragma once
#include <string_view>
#include <variant>

namespace ast::nodes {
    enum class intrinsic_types {
        i8, i16, i32, i64,
        u8, u16, u32, u64,
        f32, f64,

        infer_type
    };

    enum class operator_type {
        add, sub, mul, div, mod,
        eq, neq, lt, gt, lte, gte,
        and_, or_, not_,
        assign
    };

    struct value_type {
        std::variant<intrinsic_types, std::string_view> type;
    };
}
