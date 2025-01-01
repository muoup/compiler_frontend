#pragma once

#include <optional>
#include <string_view>
#include <variant>
#include <unordered_map>

#include "node_interfaces.h"

namespace ast::nodes {
    struct initialization;

    enum class intrinsic_type {
        i8, i16, i32, i64,
        u8, u16, u32, u64,
        f32, f64,

        char_, bool_, void_,

        init_list,

        var_args,

        infer_type
    };

    enum class bin_op_type {
        add, sub, mul, div, mod,
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

        plus_eq, minus_eq, mul_eq, div_eq, mod_eq,
        b_and_eq, b_or_eq, b_xor_eq, shl_eq, shr_eq
    };

    const static std::unordered_map<intrinsic_type, size_t> intrinsic_size {
            { intrinsic_type::i8, 1 },
            { intrinsic_type::i16, 2 },
            { intrinsic_type::i32, 4 },
            { intrinsic_type::i64, 8 },
            { intrinsic_type::u8, 1 },
            { intrinsic_type::u16, 2 },
            { intrinsic_type::u32, 4 },
            { intrinsic_type::u64, 8 },
            { intrinsic_type::f32, 4 },
            { intrinsic_type::f64, 8 },
            { intrinsic_type::char_, 1 },
            { intrinsic_type::bool_, 1 },
            { intrinsic_type::void_, 0 },
            { intrinsic_type::init_list, 0 },
            { intrinsic_type::infer_type, 0 }
    };

    struct variable_type : printable {
        NODENAME("VARIABLE_TYPE");
        DETAILS(type_str());

        std::variant<intrinsic_type, std::string_view> type;
        bool is_var_ref = false, is_const, is_volatile;

        int array_length = 0;

        // May god help anyone who needs more than 255 levels of pointer indirection
        uint8_t pointer_depth = 0;

        variable_type(std::variant<intrinsic_type, std::string_view> type, bool is_const = false, bool is_volatile = false, uint8_t pointer_depth = 0, int array_length = 0)
            : type(type), is_const(is_const), is_volatile(is_volatile), pointer_depth(pointer_depth), array_length(array_length) {}
        ~variable_type() override = default;

        static variable_type void_type() {
            return {intrinsic_type::void_ };
        }

        variable_type pointer_to() const;
        variable_type dereference() const;
        variable_type change_var_ref(bool new_val) const;
        bool is_intrinsic() const;
        bool is_pointer() const;
        bool is_int() const;
        bool is_signed() const;
        bool is_fp() const;
        size_t get_size() const;
        bool operator ==(const variable_type &other) const;
        std::string type_str() const;
    };

    struct type_instance : printable {
        NODENAME("TYPE_INSTANCE");
        DETAILS(var_name);

        variable_type type;
        std::string_view var_name;

        type_instance(variable_type type, std::string_view var_name)
            : type(type), var_name(var_name) {}

        ~type_instance() override = default;
    };

    struct method_params {
        std::vector<initialization> data;
        bool is_var_args = false;
    };
}
