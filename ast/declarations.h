#pragma once

#include <optional>
#include <string_view>
#include <vector>

namespace lex {
    struct lex_token;
    enum class lex_type;
}

namespace ast {
    using lex_cptr = std::vector<lex::lex_token>::const_iterator;

    template <typename T>
    using parse_fn = T(*)(lex_cptr&, lex_cptr);

    template <typename T>
    using loop_fn = std::optional<T>(*)(lex_cptr&);

    using parse_pred = bool(*)(lex_cptr);
}
