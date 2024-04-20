#include "util.h"

#include <algorithm>
#include <format>
#include <stdexcept>

#include "../lexer/lex.h"

using namespace ast;

std::vector<std::unordered_map<std::string_view, ast::nodes::value_type>> ast::scope_stack {};
std::unordered_map<std::string_view, std::vector<ast::nodes::type_instance>> ast::struct_types {};
std::unordered_map<std::string_view, ast::nodes::value_type> ast::function_types {};

std::optional<ast::nodes::value_type> ast::get_variable_type(std::string_view var_name) {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
        auto find = it->find(var_name);
        if (find != it->end())
            return find->second;
    }

    return std::nullopt;
}

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
    if (std::ranges::find(types, ptr->type) != types.end())
        throw_unexpected(*ptr, "Wrong Type!");

    return ptr++;
}

lex_cptr ast::assert_token_val(lex_cptr& ptr, const std::string_view val) {
    if (ptr->span != val)
        throw_unexpected(*ptr, std::format("Wrong Value! Expected: {}", val));

    return ptr++;
}

lex_cptr ast::assert_token_val(lex_cptr& ptr, const std::span<const std::string_view> vals) {
    if (std::ranges::find(vals, ptr->span) != vals.end())
        throw_unexpected(*ptr, "Wrong Value!");

    return ptr++;
}

lex_cptr ast::assert_token(lex_cptr& ptr, const parse_pred pred) {
    if (!pred(ptr))
        throw_unexpected(*ptr, "Condition not met!");

    return ptr++;
}

lex_cptr ast::consume(lex_cptr &ptr, const lex_cptr end) {
    if (ptr == end)
        throw std::runtime_error("Unexpected end of input!");

    return ptr++;
}

lex_cptr ast::peek(const lex_cptr ptr, const lex_cptr end, size_t offset) {
    if (ptr + offset >= end)
        throw std::runtime_error("Unexpected end of input!");

    return ptr + offset;
}

std::optional<lex_cptr> ast::test_token_val(lex_cptr &ptr, const std::string_view val) {
    if (ptr->span != val)
        return std::nullopt;

    return ptr++;
}

std::optional<lex_cptr> ast::test_token_val(lex_cptr &ptr, const std::span<const std::string_view> vals) {
    if (std::ranges::find(vals, ptr->span) != vals.end())
        return std::nullopt;

    return ptr++;
}

std::optional<lex_cptr> ast::test_token_type(lex_cptr &ptr, const lex::lex_type type) {
    if (ptr->type != type)
        return std::nullopt;

    return ptr++;
}

std::optional<lex_cptr> ast::test_token_type(lex_cptr &ptr, const std::span<const lex::lex_type> types) {
    if (std::ranges::find(types, ptr->type) != types.end())
        return std::nullopt;

    return ptr++;
}

std::optional<lex_cptr> ast::find_by_tok_val(const lex_cptr start, const lex_cptr end, const std::string_view val) {
    auto find = std::find_if(start, end, [val](const lex::lex_token& token) {
        return token.span == val;
    });

    if (find == end)
        return std::nullopt;

    return find;
}

std::optional<lex_cptr> ast::find_by_tok_type(const lex_cptr start, const lex_cptr end, const lex::lex_type type) {
    auto find = std::find_if(start, end, [type](const lex::lex_token& token) {
        return token.type == type;
    });

    if (find == end)
        return std::nullopt;

    return find;
}

bool ast::is_variable_identifier(const lex_cptr token) {
    return token->type == lex::lex_type::IDENTIFIER
        || token->type == lex::lex_type::PRIMITIVE;
}