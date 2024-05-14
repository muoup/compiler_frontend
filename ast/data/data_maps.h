#pragma once
#include <unordered_map>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include "abstract_data.h"

namespace ast::pm {
    const extern std::unordered_map<nodes::bin_op_type, uint16_t> binop_prec;
    const extern std::unordered_map<std::string_view, nodes::bin_op_type> binop_type_map;
    const extern std::unordered_map<std::string_view, nodes::un_op_type> unop_type_map;
    const extern std::unordered_map<std::string_view, nodes::assn_type> assign_type_map;

    const extern std::unordered_map<std::string_view, nodes::intrinsic_type> intrin_map;

    template<typename T, typename U>
    std::optional<U> find_element(const std::unordered_map<T, U> &map, const T &key) {
        const auto it = map.find(key);

        return it != map.end() ?
            std::make_optional(it->second) :
            std::nullopt;
    }

    template<typename T, typename U>
    std::optional<T> find_key(const std::unordered_map<T, U> &map, const U &value) {
        for (const auto &[key, val] : map) {
            if (val == value)
                return std::make_optional(key);
        }

        return std::nullopt;
    }
}
