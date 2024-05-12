#pragma once
#include <optional>
#include <string_view>
#include <variant>
#include "node_interfaces.h"

namespace ast::nodes {
    enum class intrinsic_types {
        i8, i16, i32, i64,
        u8, u16, u32, u64,
        f32, f64,

        char_, bool_, void_,

        init_list,

        infer_type
    };

    enum class bin_op_type {
        add, sub, mul, div, mod, pow,
        b_and, b_or, b_xor, shl, shr,
        l_and, l_or, l_xor,

        eq, neq, lt, gt, lte, gte,

        acc, accdf,
    };

    enum class un_op_type {
        deref, addr_of, bit_not, log_not, negate
    };

    enum class assn_type {
        none,

        plus_eq, minus_eq, mul_eq, div_eq, mod_eq, pow_eq,
        b_and_eq, b_or_eq, b_xor_eq, shl_eq, shr_eq
    };

    struct variable_type : printable {
        NODENAME("VARIABLE_TYPE");
        DETAILS(type_str());

        std::variant<intrinsic_types, std::string_view> type;
        bool is_const, is_volatile;

        // May god help anyone who needs more than 255 levels of pointer indirection
        uint8_t pointer_depth = 0;

        variable_type(std::variant<intrinsic_types, std::string_view> type, bool is_const = false, bool is_volatile = false, uint8_t pointer_depth = 0)
            : type(type), is_const(is_const), is_volatile(is_volatile), pointer_depth(pointer_depth) {}
        ~variable_type() override = default;

        static variable_type void_type() {
            return { intrinsic_types::void_ };
        }

        variable_type pointer_to() const;
        variable_type dereference() const;
        bool is_intrinsic() const;
        bool is_pointer() const;
        bool is_fp() const;
        bool operator ==(const variable_type &other) const;
        std::string_view type_str() const;
    };

    struct type_instance : printable {
        NODENAME("TYPE_INSTANCE");
        CHILDREN(type);

        variable_type type;
        std::string_view var_name;

        type_instance(variable_type type, std::string_view var_name)
            : type(type), var_name(var_name) {}

        ~type_instance() override = default;
    };
}
