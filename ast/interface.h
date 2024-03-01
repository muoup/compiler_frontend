#pragma once
#include <vector>
#include "declarations.h"

namespace lex {
    struct lex_token;
}

namespace ast {
    ast_node parse(const std::vector<lex::lex_token> &tokens);
}
