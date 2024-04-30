#include "operator.h"
#include "../data/data_maps.h"
#include "../util.h"

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

    int16_t get_prec(const nodes::bin_op_type& op) {
        if (const auto find = binop_prec.find(op); find != binop_prec.end())
            return find->second;

        return 0;
    }

    std::unique_ptr<nodes::expression> load_if_necessary(std::unique_ptr<nodes::expression> node) {
        if (dynamic_cast<nodes::var_ref*>(node.get()) != nullptr)
            return std::make_unique<nodes::load>(std::move(node));

        return node;
    }

    nodes::bin_op create_bin_op(std::unique_ptr<nodes::expression> left, std::unique_ptr<nodes::expression> right, nodes::bin_op_type type) {
        const auto l_type = left->get_type();
        const auto r_type = right->get_type();

        if (type != nodes::bin_op_type::acc && type != nodes::bin_op_type::accdf) {
            left = load_if_necessary(std::move(left));
            right = load_if_necessary(std::move(right));
        } else if (type == nodes::bin_op_type::accdf){
            left = std::make_unique<nodes::load>(std::move(left));
        }

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
                nodes::value_type {
                    l_type
                }
            )
        };
    }

    nodes::assignment create_assignment(std::unique_ptr<nodes::expression> left, std::unique_ptr<nodes::expression> right, std::optional<nodes::bin_op_type> additional_operator) {
        auto l_type = left->get_type();
        auto r_type = right->get_type();

        if (l_type == r_type) {
            if (!additional_operator.has_value() || l_type.is_intrinsic() && r_type.is_intrinsic()) {
                return nodes::assignment {
                    std::move(left),
                    std::move(right),
                    additional_operator
                };
            }

            throw std::runtime_error("Non-intrinsic types cannot be casted (yet)!");
        }

        if (!l_type.is_intrinsic() && ast::instance_of<nodes::initializer_list>(right.get())) {
            auto *initializer_list = dynamic_cast<nodes::initializer_list*>(right.get());

            auto struct_initializer = nodes::struct_initializer {
                std::get<std::string_view>(l_type.type),
                std::move(initializer_list->values)
            };

            return nodes::assignment {
                std::move(left),
                std::make_unique<nodes::struct_initializer>(std::move(struct_initializer)),
                additional_operator
            };
        }

        return nodes::assignment {
            std::move(left),
            std::make_unique<nodes::cast>(
                std::move(right),
                nodes::value_type {
                    l_type
                }
            ),
            additional_operator
        };
    }
}