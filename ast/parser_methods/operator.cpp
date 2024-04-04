#include "operator.h"
#include "../data/data_maps.h"

namespace ast::pm {
    const auto find = []<typename T, typename Key>(const std::unordered_map<Key, T> map, const Key& key) -> std::optional<T> {
        if (auto find = map.find(key); find != map.end())
            return std::make_optional(find->second);

        return std::nullopt;
    };

    std::optional<nodes::un_op_type> get_unop(const std::string_view op) {
        return find(unop_type_map, op);
    }

    std::optional<nodes::bin_op_type> get_binop(const std::string_view op) {
        return find(binop_type_map, op);
    }

    std::optional<nodes::assn_type> get_assign(std::string_view op) {
        return find(assign_type_map, op);
    }

    std::string_view from_binop(const nodes::bin_op_type& by_type) {
        for (auto [key, type] : binop_type_map)
            if (type == by_type)
                return key;

        return "OPERATOR NOT FOUND";
    }

    std::string_view from_unop(const nodes::un_op_type& by_type) {
        for (auto [key, type] : unop_type_map)
            if (type == by_type)
                return key;

        return "OPERATOR NOT FOUND";
    }

    int16_t get_prec(const nodes::bin_op& op) {
        if (const auto find = binop_prec.find(op.type); find != binop_prec.end())
            return find->second;

        return 0;
    }
}