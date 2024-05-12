#pragma once
#include <cstdint>
#include <functional>

#include "../data/ast_nodes.h"

namespace ast::pm {
    std::unique_ptr<nodes::expression> load_if_necessary(std::unique_ptr<nodes::expression> node);

    nodes::bin_op create_bin_op(std::unique_ptr<nodes::expression> left, std::unique_ptr<nodes::expression> right, nodes::bin_op_type type);
    nodes::assignment create_assignment(std::unique_ptr<nodes::expression> left, std::unique_ptr<nodes::expression> right, std::optional<nodes::bin_op_type> additional_operator);
}
