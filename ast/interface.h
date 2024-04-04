#pragma once

#include <vector>
#include "declarations.h"
#include "data/ast_nodes.h"

namespace lex {
    struct lex_token;
}

namespace ast {
    nodes::root parse(const std::vector<lex::lex_token> &tokens);
}
