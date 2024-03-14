#pragma once
#include <vector>
#include "data/ast_nodes.h"
#include "declarations.h"

namespace lex {
    struct lex_token;
}

namespace ast {
    nodes::root parse(const std::vector<lex::lex_token> &tokens);
}
