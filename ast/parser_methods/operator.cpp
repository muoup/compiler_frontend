#include "operator.h"
#include "../util.h"

namespace ast::pm {
    const auto find = []<typename T, typename Key>(const std::unordered_map<Key, T> map, const Key& key) -> std::optional<T> {
        if (auto find = map.find(key); find != map.end())
            return std::make_optional(find->second);

        return std::nullopt;
    };

    nodes::bin_op create_bin_op(std::unique_ptr<nodes::expression> left, std::unique_ptr<nodes::expression> right, nodes::bin_op_type type) {
        const auto l_type = left->get_type();
        const auto r_type = right->get_type();

        if (l_type == r_type) {
            return nodes::bin_op {
                    type,
                    std::move(left),
                    std::move(right),
            };
        }

        if (!l_type.is_intrinsic() || !r_type.is_intrinsic())
            throw std::runtime_error("Non-intrinsic types cannot be casted (yet)!");

        return nodes::bin_op {
            type,
            std::move(left),
            std::make_unique<nodes::cast>(
                std::move(right),
                nodes::variable_type {
                    l_type
                }
            )
        };
    }

    nodes::assignment create_assignment(std::unique_ptr<nodes::expression> left, std::unique_ptr<nodes::expression> right, std::optional<nodes::bin_op_type> additional_operator) {
        auto l_type = left->get_type();
        auto r_type = right->get_type();

        return nodes::assignment {
            std::move(left),
            std::move(right),
            additional_operator
        };
    }
}