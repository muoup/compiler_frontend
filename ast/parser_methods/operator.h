#pragma once
#include <cstdint>
#include <functional>
#include <string_view>
#include <unordered_map>

#include "../data/ast_nodes.h"

namespace ast::pm {
    using binop_fn = std::function<int(int, int)>;

    const extern std::unordered_map<nodes::operator_type, uint16_t> binop_prec;
    const extern std::unordered_map<nodes::operator_type, binop_fn> binop_fn_map;
    const extern std::unordered_map<std::string_view, nodes::operator_type> binop_type_map;

    nodes::operator_type get_op(std::string_view op);
    int16_t get_prec(nodes::op op);
}
