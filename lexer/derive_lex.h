#pragma once
#include <optional>
#include <string_view>

#include "lex.h"

namespace lex {
    struct lex_token;
    enum class lex_type;

    struct derived_lex {
        lex_type type;
        std::string_view span;
        str_ptr end;

        derived_lex(const lex_type type, str_ptr start, str_ptr end, const ptrdiff_t skip_chars = 0)
            : type(type), span(start, end + 1), end(end + skip_chars) { }
    };

    std::optional<derived_lex> derive_strlit(str_ptr start, str_ptr end);

    std::optional<derived_lex> derive_charlit(str_ptr start, str_ptr end);
    lex_token gen_numeric(str_ptr start, str_ptr end);

    std::optional<derived_lex> derive_expr_op(str_ptr start, str_ptr end);
    std::optional<derived_lex> derive_assn_op(str_ptr start, str_ptr end);
    std::optional<derived_lex> derive_punctuator(str_ptr start, str_ptr end);
}
