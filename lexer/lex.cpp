#include "lex.h"

#include <cctype>
#include <ranges>

using namespace lex;

struct derived_lex {
    naive_type type;
    std::string_view span;
    std::string_view::const_iterator end = span.cend() - 1;
};

bool
ident_end(const char c) {
    return SYMBOL_SET.contains(c) || c == ' ' || c == '\n';
}

std::optional<derived_lex>
derive_numeric(const std::string_view span) {
    naive_type type = naive_type::INT_LITERAL;
    auto ptr = span.cbegin();

    for (; ptr != span.end(); ptr++) {
        if (*ptr == '.' && type == naive_type::INT_LITERAL)
            type = naive_type::FLOAT_LITERAL;
        else if (!isdigit(*ptr))
            return std::nullopt;
        else if (*ptr == ' ' || *ptr == '\n')
            break;
    }

    return derived_lex { type, { span.cbegin(), ptr } };
}

std::optional<derived_lex>
derive_strlit(const std::string_view span) {
    if (span.front() != '\"')
        return std::nullopt;

    const auto end = std::ranges::find(std::string_view{ span.cbegin() + 1, span.cend() }, '\"');

    if (end == span.cend())
        throw std::runtime_error("Unterminated string literal");

    return derived_lex { naive_type::STRING_LITERAL, { span.cbegin() + 1, end - 1 }, end };
}

std::optional<derived_lex>
derive_charlit(const std::string_view span) {
    if (span.front() != '\'')
        return std::nullopt;

    if (span.at(2) != '\'')
        throw std::runtime_error("Unclosed or invalid character literal");

    return derived_lex { naive_type::CHAR_LITERAL, { span.cbegin() + 1 }, span.cbegin() + 2 };
}

std::optional<derived_lex>
derived_nonlit(const std::string_view span) {
    const auto end = std::ranges::find_if(span, ident_end);
    const std::string_view ident { span.cbegin(), end };

    if (KEYWORD_SET.contains(ident))
        return derived_lex { naive_type::KEYWORD, ident };

    return derived_lex { naive_type::IDENTIFIER, ident };
}

std::optional<derived_lex>
derive_ident(const std::string_view span) {
    if (span.empty())
        return std::nullopt;

    return derive_strlit(span)
        .or_else([span] { return derive_charlit(span); })
        .or_else([span] { return derive_numeric(span); })
        .or_else([span] { return derived_nonlit(span); });
}

std::optional<derived_lex>
derive_operator(const std::string_view span) {
    if (SPECIAL_SYMBOL.contains({ span.cbegin(), span.cbegin() + 2 }))
        return derived_lex { naive_type::SYMBOL, { span.cbegin(), span.cbegin() + 2 } };
    else
        return derived_lex { naive_type::SYMBOL, { span.cbegin(), span.cbegin() + 1 } };
}

std::vector<naive_token> lex::naive_lex(const std::string_view code) {
    std::vector<naive_token> tokens;

    for (auto ptr = code.cbegin(); ptr != code.cend(); ++ptr) {
        if (*ptr == ' ' || *ptr == '\n') continue;

        const auto derive = SYMBOL_SET.contains(*ptr) ?
            derive_operator({ ptr, code.cend() }) :
            derive_ident({ ptr, code.cend() });

        if (derive == std::nullopt)
            continue;

        const auto [type, span, end] = derive.value();
        tokens.push_back({ type, span });
        ptr = end;
    }

    return tokens;
}
