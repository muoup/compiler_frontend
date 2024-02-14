#include "derive_lex.h"

#include <format>
#include <stdexcept>
#include <string_view>

#include "lex.h"

using namespace lex;

bool
ident_end(const char c) {
    return SYMBOL_SET.contains(c) || c == ' ' || c == '\n';
}


lex_token
lex::generate_numeric(const std::string_view span) {
    lex_type type = lex_type::INT_LITERAL;
    auto ptr = span.cbegin();

    for (; ptr != span.end(); ptr++) {
        if (*ptr == '.' && type == lex_type::INT_LITERAL)
            type = lex_type::FLOAT_LITERAL;
        else if (!isdigit(*ptr))
            throw std::runtime_error(std::format("Invalid numerical literal: {}", span));
    }

    return { type, span };
}

std::optional<derived_lex>
lex::derive_strlit(const std::string_view span) {
    if (span.front() != '\"')
        return std::nullopt;

    const auto end = std::ranges::find(std::string_view{ span.cbegin() + 1, span.cend() }, '\"');

    if (end == span.cend())
        throw std::runtime_error("Unterminated string literal");

    return derived_lex { lex_type::STRING_LITERAL, { span.cbegin() + 1, end - 1 }, end };
}

std::optional<derived_lex>
lex::derive_charlit(const std::string_view span) {
    if (span.front() != '\'')
        return std::nullopt;

    if (span.at(2) != '\'')
        throw std::runtime_error("Unclosed or invalid character literal");

    return derived_lex { lex_type::CHAR_LITERAL, { span.cbegin() + 1 }, span.cbegin() + 2 };
}

std::optional<derived_lex>
lex::derive_operator(const std::string_view span) {
    if (!SYMBOL_SET.contains(span.front()))
        return std::nullopt;

    if (SPECIAL_SYMBOL.contains({ span.cbegin(), span.cbegin() + 2 }))
        return derived_lex { lex_type::SYMBOL, { span.cbegin(), span.cbegin() + 2 } };
    else
        return derived_lex { lex_type::SYMBOL, { span.cbegin(), span.cbegin() + 1 } };
}

std::optional<derived_lex>
lex::derive_punctuator(const std::string_view span) {
    if (!PUNCTUATOR_SET.contains(span.front()))
        return std::nullopt;

    return derived_lex { lex_type::PUNCTUATOR, { span.cbegin(), span.cbegin() + 1 } };
}