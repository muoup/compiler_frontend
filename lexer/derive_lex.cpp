#include "derive_lex.h"

#include <format>
#include <stdexcept>

#include "lex.h"

using namespace lex;

bool
ident_end(const char c) {
    return SYMBOL_SET.contains(c) || c == ' ' || c == '\n';
}

lex_token
lex::gen_numeric(const str_ptr start, const str_ptr end) {
    lex_type type = lex_type::INT_LITERAL;
    auto ptr = start;

    for (; ptr != end; ++ptr) {
        if (*ptr == '.' && type == lex_type::INT_LITERAL)
            type = lex_type::FLOAT_LITERAL;
        else if (!isdigit(*ptr))
            throw std::runtime_error(std::format("Invalid numerical literal: {}", std::string_view { ptr, end }));
    }

    return { type, { start, end } };
}

std::optional<derived_lex>
lex::derive_strlit(const str_ptr start, const str_ptr end) {
    if (*start != '\"')
        return std::nullopt;

    auto find = start + 1;

    while (find != end && *find != '\"' && find[-1] != '\\')
        ++find;

    if (find == end)
        throw std::runtime_error("Unterminated string literal");

    return derived_lex { lex_type::STRING_LITERAL, start + 1, find - 1, 1 };
}

std::optional<derived_lex>
lex::derive_charlit(const str_ptr start, const str_ptr end) {
    if (*start != '\'')
        return std::nullopt;

    const bool is_escaped = start[1] == '\\';
    const auto expected_end = 2 + is_escaped;

    if (end - start < 2 || start[expected_end] != '\'')
        throw std::runtime_error("Unclosed or invalid character literal");

    return derived_lex { lex_type::CHAR_LITERAL, start + 1, start + expected_end - 1, 1 };
}

std::optional<derived_lex>
lex::derive_operator(const str_ptr start, const str_ptr end) {
    if (!SYMBOL_SET.contains(*start))
        return std::nullopt;

    const bool is_special = start <= end - 2 && SPECIAL_SYMBOL.contains({ start, start + 2 });

    return derived_lex { lex_type::SYMBOL, start, start + is_special };
}

std::optional<derived_lex>
lex::derive_punctuator(const str_ptr start, const str_ptr) {
    if (!PUNCTUATOR_SET.contains(*start))
        return std::nullopt;

    return derived_lex { lex_type::PUNCTUATOR, start, start };
}