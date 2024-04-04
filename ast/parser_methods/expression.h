#pragma once

#include <memory>
#include "../declarations.h"
#include "../data/node_interfaces.h"

namespace ast::nodes {
    enum class assn_type;
    enum class bin_op_type;
}

namespace ast::pm {
    std::unique_ptr<nodes::expression> parse_expression(lex_cptr &ptr, const lex_cptr end);
}
