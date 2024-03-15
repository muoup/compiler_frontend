#pragma once

#include "../data/node_defs.h"
#include "../declarations.h"

namespace ast::nodes {
    enum class bin_op_type;
}

namespace ast::pm {
    nodes::expression parse_expression(lex_cptr &ptr, const lex_cptr end);

    nodes::expression pure_assignment(nodes::expression &lhs, nodes::expression &rhs, nodes::bin_op_type op);
    nodes::expression assign_initialization(nodes::expression &lhs, nodes::expression &rhs, nodes::bin_op_type type);
}
