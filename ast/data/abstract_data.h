#pragma once
#include <optional>
#include <string_view>
#include <variant>

namespace ast::nodes {
    enum class intrinsic_types {
        i8, i16, i32, i64,
        u8, u16, u32, u64,
        f32, f64,

        char_, bool_, void_,

        infer_type
    };

    enum class bin_op_type {
        add, sub, mul, div, mod, pow,
        b_and, b_or, b_xor, shl, shr,
        l_and, l_or, l_xor,

        eq, neq, lt, gt, lte, gte,

        dot, arrow
    };

    enum class un_op_type {
        deref, addr_of, bit_not, log_not, negate
    };

    enum class assn_type {
        none,

        plus_eq, minus_eq, mul_eq, div_eq, mod_eq, pow_eq,
        b_and_eq, b_or_eq, b_xor_eq, shl_eq, shr_eq
    };

    enum var_type_category {
        INTRINSIC,
        NON_INTRINSIC
    };

    struct value_type {
        std::variant<intrinsic_types, std::string_view> type;
        bool is_const, is_volatile;

        void print(size_t depth) const;
        bool is_intrinsic() const {
            return std::holds_alternative<intrinsic_types>(type);
        };

        static value_type void_type() {
            return { intrinsic_types::void_ };
        }
    };

    struct type_instance {
        value_type type;
        std::string_view var_name;

        void print(size_t depth) const;
    };

    std::optional<intrinsic_types> get_intrinsic_type(std::string_view type);
}
