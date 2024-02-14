#pragma once
#include <optional>
#include <string_view>

namespace lex {
    struct lex_token;
    enum class lex_type;

    struct derived_lex {
        lex_type type;
        std::string_view span;
        std::string_view::const_iterator end = span.cend() - 1;
    };

    struct maybe_derive {
        std::optional<derived_lex> val;

        static maybe_derive create() {
            return maybe_derive { std::nullopt };
        }
    };

    std::optional<derived_lex> derive_strlit(std::string_view span);
    std::optional<derived_lex> derive_charlit(std::string_view span);
    std::optional<derived_lex> derive_operator(std::string_view span);
    std::optional<derived_lex> derive_punctuator(std::string_view span);
    lex_token generate_numeric(std::string_view span);
}
