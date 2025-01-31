#pragma once

#include <optional>
#include <span>
#include <stdexcept>
#include <vector>
#include <unordered_map>

#include "util.h"
#include "data/abstract_data.h"
#include "data/ast_nodes.h"

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

    extern std::vector<std::unordered_map<std::string_view, ast::nodes::variable_type>> scope_stack;
    extern std::unordered_map<std::string_view, std::vector<ast::nodes::type_instance>> struct_types;
    extern std::unordered_map<std::string_view, ast::nodes::function_prototype*> function_prototypes;

    extern std::vector<ast::nodes::method_call*> unfinished_method_calls;

    extern ast::nodes::function_prototype const* current_function;

    std::optional<ast::nodes::variable_type> get_var_type(std::string_view var_name);

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

    lex_cptr consume(lex_cptr &ptr, lex_cptr end);
    lex_cptr peek(lex_cptr ptr, lex_cptr end, size_t offset = 0);

    bool try_peek_type(lex_cptr ptr, lex_cptr end, lex::lex_type type, size_t offset = 0);
    bool try_peek_val(lex_cptr ptr, lex_cptr end, std::string_view val, size_t offset = 0);

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
                    throw std::runtime_error("Expected but never found: {}" + std::string(until));
                return end;
            }).value();

        auto ret_node = fn(ptr, terminate);
        ptr = terminate + 1;

        return ret_node;
    }

    template <typename T, typename lex_cptr>
    T parse_between(lex_cptr& ptr, const parse_fn<T> fn) {
        if (!ptr->closer)
            throw std::runtime_error("Tried to parse between a token with no closer");

        const auto end = ptr->closer.value();
        auto node = fn(++ptr, end);

        ptr = end + 1;

        return node;
    }

    template <typename T, typename lex_cptr>
    T parse_between(lex_cptr& ptr, std::string_view val, const parse_fn<T> fn) {
        if (ptr->span != val)
            throw std::runtime_error("Expected: {}" + std::string(ptr->span));

        return parse_between(ptr, fn);
    }

    template <typename T, typename lex_cptr>
    std::vector<T> parse_split(lex_cptr& ptr, const lex_cptr end, const std::string_view split_val, const parse_fn<T> fn) {
        std::vector<T> split;

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

    template <typename T, typename U>
    bool instance_of(const U* val) {
        return dynamic_cast<const T*>(val) != nullptr;
    }
}
