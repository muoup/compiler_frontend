#pragma once
#include <cstdint>
#include <functional>

#include "../data/ast_nodes.h"

namespace ast::pm {
    using binop_fn = std::function<int(int, int)>;

    const extern std::unordered_map<nodes::bin_op_type, uint16_t> binop_prec;
    const extern std::unordered_map<nodes::un_op_type, binop_fn> binop_fn_map;
    const extern std::unordered_map<std::string_view, nodes::bin_op_type> binop_type_map;
    const extern std::unordered_map<std::string_view, nodes::un_op_type> unop_type_map;

    nodes::un_op_type get_unop(std::string_view op);
    nodes::bin_op_type get_binop(std::string_view op);

    std::string_view from_unop(const nodes::un_op_type& by_type);
    std::string_view from_binop(const nodes::bin_op_type& by_type);

    int16_t get_prec(const nodes::bin_op& op);
}
