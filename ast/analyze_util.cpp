#include "analyze_util.h"

#include <algorithm>
#include <format>
#include <stdexcept>
#include <string_view>
#include "../lexer/lex.h"

using namespace ast;

void ast::throw_unexpected(const lex::lex_token& token, const std::string_view expected) {
    throw std::runtime_error(std::format("Unexpected token: {}, {}", token.span, expected));
}

void ast::throw_unclosed(const lex::lex_token& token, const std::string_view expected) {
    throw std::runtime_error(std::format("Unclosed token: {}, {}", token.span, expected));
}

lex_cptr ast::assert_token_type(lex_cptr& ptr, const lex::lex_type type) {
    if (ptr->type != type)
        throw_unexpected(*ptr, "Wrong Type!");

    return ptr++;
}

lex_cptr ast::assert_token_type(lex_cptr& ptr, const std::span<const lex::lex_type> types) {
    if (!std::ranges::contains(types, ptr->type))
        throw_unexpected(*ptr, "Wrong Type!");

    return ptr++;
}

lex_cptr ast::assert_token_val(lex_cptr& ptr, const std::string_view val) {
    if (ptr->span != val)
        throw_unexpected(*ptr, std::format("Wrong Value! Expected: {}", val));

    return ptr++;
}

lex_cptr ast::assert_token_val(lex_cptr& ptr, const std::span<const std::string_view> vals) {
    if (!std::ranges::contains(vals, ptr->span))
        throw_unexpected(*ptr, "Wrong Value!");

    return ptr++;
}

bool ast::test_token_val(lex_cptr &ptr, const std::string_view val) {
    if (ptr->span != val)
        return false;

    ++ptr;
    return true;
}

bool ast::test_token_val(lex_cptr &ptr, const std::span<const std::string_view> vals) {
    if (!std::ranges::contains(vals, ptr->span))
        return false;

    ++ptr;
    return true;
}

bool ast::test_token_type(lex_cptr &ptr, const lex::lex_type type) {
    if (ptr->type != type)
        return false;

    ++ptr;
    return true;
}

bool ast::test_token_type(lex_cptr &ptr, const std::span<const lex::lex_type> types) {
    if (!std::ranges::contains(types, ptr->type))
        return false;

    ++ptr;
    return true;
}

std::optional<lex_ptr> ast::find_by_tok_val(const lex_ptr start, const lex_ptr end, const std::string_view val) {
    const auto find = std::find_if(start, end, [val](const lex::lex_token& token) {
        return token.span == val;
    });

    if (find == end)
        return std::nullopt;

    return find;
}

std::optional<lex_ptr> ast::find_by_tok_type(const lex_ptr start, const lex_ptr end, const lex::lex_type type) {
    const auto find = std::find_if(start, end, [type](const lex::lex_token& token) {
        return token.type == type;
    });

    if (find == end)
        return std::nullopt;

    return find;
}