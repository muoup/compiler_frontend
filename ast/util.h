#pragma once

#include <format>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

#include "util.h"

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

    void throw_unexpected(const lex::lex_token& token, std::string_view expected = "No explanation given.");
    void throw_unclosed(const lex::lex_token& token, std::string_view expected = "No explanation given.");

    lex_cptr assert_token_type(lex_cptr& ptr, lex::lex_type type);
    lex_cptr assert_token_type(lex_cptr& ptr, std::span<const lex::lex_type> types);
    lex_cptr assert_token_val(lex_cptr& ptr, std::string_view val);
    lex_cptr assert_token_val(lex_cptr& ptr, std::span<const std::string_view> vals);

    lex_cptr assert_token(lex_cptr& ptr, parse_pred pred);

    template <typename T>
    T unwrap_or_throw(std::optional<T> opt, std::string_view msg = "Expected value but got nullopt!") {
        if (!opt)
            throw std::runtime_error(msg.data());

        return opt.value();
    }

    std::optional<lex_cptr> test_token_val(lex_cptr &ptr, std::string_view val);
    std::optional<lex_cptr> test_token_val(lex_cptr &ptr, std::span<const std::string_view> vals);
    std::optional<lex_cptr> test_token_type(lex_cptr &ptr, lex::lex_type type);
    std::optional<lex_cptr> test_token_type(lex_cptr &ptr, std::span<const lex::lex_type> types);

    std::optional<lex_cptr> find_by_tok_val(lex_cptr start, lex_cptr end, std::string_view val);
    std::optional<lex_cptr> find_by_tok_type(lex_cptr start, lex_cptr end, lex::lex_type type);

    bool is_variable_identifier(lex_cptr token);

    template <typename T, typename lex_cptr>
    T parse_until(lex_cptr &ptr, lex_cptr end, std::string_view until, const parse_fn<T> fn,
                                      const bool assert_contains = true) {
        const auto terminate = find_by_tok_val(ptr, end, until)
            .or_else([assert_contains, end, &until] -> std::optional<lex_cptr> {
                if (assert_contains)
                    throw std::runtime_error(std::format("Expected but never found: {}", until));
                return end;
            }).value();

        auto ret_node = fn(ptr, terminate);
        ptr = terminate + 1;

        return ret_node;
    }

    template <typename T, typename lex_cptr>
    T parse_between(lex_cptr& ptr, const parse_fn<T> fn) {
        if (!ptr->closer)
            throw std::runtime_error(std::format("Tried to parse between a token with no closer. Token: {}", ptr->span));

        const auto end = ptr->closer.value();
        auto node = fn(++ptr, end);

        ptr = end + 1;

        return node;
    }

    template <typename T, typename lex_cptr>
    T parse_between(lex_cptr& ptr, std::string_view val, const parse_fn<T> fn) {
        if (ptr->span != val)
            throw std::runtime_error(std::format("Expected: {} but got: {}", val, ptr->span));

        return parse_between(ptr, fn);
    }

    template <typename T, typename lex_cptr>
    std::vector<T> parse_split(lex_cptr& ptr, const lex_cptr end, const std::string_view split_val, const parse_fn<T> fn) {
        std::vector<T> split;

        // ++ptr;

        while (ptr < end) {
            split.emplace_back(
                parse_until<T, lex_cptr>(ptr, end, split_val, fn, false)
            );
        }

        return split;
    }

    template <typename T, typename lex_cptr>
    std::optional<T> try_parse(lex_cptr &ptr, const lex_cptr end, const parse_fn<T> fn) {
        const lex_cptr start_cache = ptr;

        try {
            return fn(ptr, end);
        } catch (const std::runtime_error&) {
            ptr = start_cache;
            return std::nullopt;
        }
    }

    template <typename T, typename lex_cptr>
    std::vector<T> capture_contiguous(lex_cptr& ptr, const lex_cptr end, const loop_fn<T> fn) {
        std::vector<T> nodes;

        while (auto node = ptr < end ? fn(ptr) : std::nullopt)
            nodes.emplace_back(node.value());

        return nodes;
    }

    template <typename T, typename lex_cptr>
    T try_parse(lex_cptr &ptr, const lex_cptr end, const parse_fn<T> fn, auto... fn_va) {
        if (auto node = try_parse(ptr, end, fn)) {
            return node.value();
        }

        if constexpr (sizeof...(fn_va) == 0) {
            throw std::runtime_error("No more functions to try!");
        }

        return ast::try_parse(ptr, fn_va...);
    }

    template <typename T, typename lex_cptr>
    loop_fn<T> test_token_predicate(parse_pred pred) {
        return [pred](lex_cptr& ptr) -> std::optional<lex_cptr> {
            if (!pred(ptr))
                return std::nullopt;

            return ptr++;
        };
    }
}
