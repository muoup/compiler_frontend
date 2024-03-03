#include "util_methods.h"

#include <algorithm>
#include <format>
#include <stdexcept>

#include "../lexer/lex.h"

using namespace ast;

ast_node ast::parse_until(lex_cptr& ptr, lex_cptr end, std::string_view until, const parse_fn fn, const bool assert_contains) {
    const auto terminate = find_by_tok_val(ptr, end, until)
        .or_else([assert_contains, end, &until] -> std::optional<lex_cptr> {
            if (assert_contains)
                throw std::runtime_error(std::format("Expected but never found: {}", until));
            return end;
        }).value();

    const auto ret_node = fn(ptr, terminate);
    ptr = terminate + 1;

    return ret_node;
}

ast_node ast::parse_between(lex_cptr& ptr, const parse_fn fn) {
    if (!ptr->closer)
        throw std::runtime_error(std::format("Tried to parse between a token with no closer. Token: {}", ptr->span));

    const auto end = ptr->closer.value();
    const auto node = fn(++ptr, end);

    ptr = end + 1;

    return node;
}

ast_node ast::parse_between(lex_cptr& ptr, const std::string_view exp_val, const parse_fn fn) {
    return parse_between(ptr = assert_token_val(ptr, exp_val), fn);
}

std::vector<ast_node> ast::parse_split(lex_cptr& ptr, const lex_cptr end, const std::string_view delimiter, const parse_fn fn) {
    std::vector<ast_node> split;

    while (ptr < end) {
        split.push_back(
            parse_until(ptr, end, delimiter, fn, false)
        );
    }

    return split;
}

std::vector<ast_node> ast::capture_contiguous(lex_cptr& ptr, const lex_cptr end, const loop_fn fn) {
    std::vector<ast_node> nodes;

    while (ptr < end) {
        if (auto node = fn(ptr)) {
            nodes.push_back(node.value());
        } else {
            break;
        }
    }

    return nodes;
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

std::optional<ast_node> ast::try_parse(lex_cptr &ptr, const lex_cptr end, const parse_fn fn) {
    const lex_cptr start_cache = ptr;

    try {
        return fn(ptr, end);
    } catch (const std::runtime_error&) {
        ptr = start_cache;
        return std::nullopt;
    }
}

ast_node ast::try_parse(lex_cptr &ptr, const lex_cptr end, const parse_fn fn, auto... fn_va) {
    if (auto node = try_parse(ptr, end, fn)) {
        return node.value();
    }

    if constexpr (sizeof...(fn_va) == 0) {
        throw std::runtime_error("No more functions to try!");
    }

    return ast::try_parse(ptr, fn_va...);
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

loop_fn ast::test_token_predicate(auto pred) {
    return [pred](lex_cptr& ptr) -> std::optional<lex_cptr> {
        if (!pred(ptr))
            return std::nullopt;

        return ptr++;
    };
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
        || token->type == lex::lex_type::KEYWORD;
}

std::optional<ast_node> ast::gen_variable_identifier(lex_cptr& ptr) {
    if (!is_variable_identifier(ptr))
        return std::nullopt;

    return ast_node {
        ast_node_type::VARIABLE,
        ptr++->span
    };
}
