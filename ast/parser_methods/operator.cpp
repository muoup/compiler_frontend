#include "operator.h"
#include "../util.h"

namespace ast::pm {
    const auto find = []<typename T, typename Key>(const std::unordered_map<Key, T> map, const Key& key) -> std::optional<T> {
        if (auto find = map.find(key); find != map.end())
            return std::make_optional(find->second);

        return std::nullopt;
    };

    std::unique_ptr<nodes::expression> load_if_necessary(std::unique_ptr<nodes::expression> node) {
        if (node->get_type().is_var_ref) {
            return std::make_unique<nodes::load>(std::move(node));
        }

        return node;
    }

    nodes::bin_op create_bin_op(std::unique_ptr<nodes::expression> left, std::unique_ptr<nodes::expression> right, nodes::bin_op_type type) {
        const auto l_type = left->get_type();
        const auto r_type = right->get_type();

        left = load_if_necessary(std::move(left));
        right = load_if_necessary(std::move(right));

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

        if (ast::instance_of<nodes::initializer_list>(right.get())) {
            if (additional_operator)
                throw std::runtime_error("Cannot use additional operator with initializer list");

            auto *initializer_list = dynamic_cast<nodes::initializer_list*>(right.get());

            if (l_type.array_length != 0) {
                if (l_type.array_length == -1) {
                    auto *var_ref = dynamic_cast<nodes::initialization*>(left.get());

                    if (!var_ref)
                        throw std::runtime_error("Cannot auto-complete array initialization for non-variable");

                    var_ref->variable.type.array_length = initializer_list->values.size();
                    l_type = var_ref->get_type();
                }

                if (l_type.array_length < initializer_list->values.size())
                    throw std::runtime_error("Initializer list is too long");

                if (l_type.array_length != initializer_list->values.size())
                    throw std::runtime_error("Compiler cannot auto-complete array initialization yet");

                right = std::make_unique<nodes::array_initializer>(
                    l_type.dereference(), std::move(initializer_list->values)
                );
            } else if (!l_type.is_intrinsic() && !l_type.is_pointer()) {
                right = std::make_unique<nodes::struct_initializer>(
                    l_type.type_str(), std::move(initializer_list->values)
                );
            } else {
                throw std::runtime_error("Cannot assign array initializer to primitive/pointer non-array type");
            }
        } else if (l_type != r_type) {
            right = std::make_unique<nodes::cast>(
                std::move(right),
                nodes::variable_type {
                    l_type
                }
            );
        }

        return nodes::assignment {
            std::move(left),
            std::move(right),
            additional_operator
        };
    }
}