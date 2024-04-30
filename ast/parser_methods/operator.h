#pragma once
#include <cstdint>
#include <functional>

#include "../data/ast_nodes.h"

namespace ast::pm {
    std::optional<nodes::un_op_type> get_unop(std::string_view op);
    std::optional<nodes::bin_op_type> get_binop(std::string_view op);
    std::optional<nodes::assn_type> get_assign(std::string_view op);

    std::string_view from_unop(const nodes::un_op_type& by_type);
    std::string_view from_binop(const nodes::bin_op_type& by_type);

    int16_t get_prec(const nodes::bin_op_type& op);

    std::unique_ptr<nodes::expression> load_if_necessary(std::unique_ptr<nodes::expression> node);

    nodes::bin_op create_bin_op(std::unique_ptr<nodes::expression> left, std::unique_ptr<nodes::expression> right, nodes::bin_op_type type);
    nodes::assignment create_assignment(std::unique_ptr<nodes::expression> left, std::unique_ptr<nodes::expression> right, std::optional<nodes::bin_op_type> additional_operator);
}
