#pragma once

#include <optional>
#include <span>
#include <string_view>
#include <vector>

#include "declarations.h"

namespace ast {
    void throw_unexpected(const lex::lex_token& token, std::string_view expected = "No explanation given.");
    void throw_unclosed(const lex::lex_token& token, std::string_view expected = "No explanation given.");

    lex_cptr assert_token_type(lex_cptr& ptr, lex::lex_type type);
    lex_cptr assert_token_type(lex_cptr& ptr, std::span<const lex::lex_type> types);
    lex_cptr assert_token_val(lex_cptr& ptr, std::string_view val);
    lex_cptr assert_token_val(lex_cptr& ptr, std::span<const std::string_view> vals);

    lex_cptr assert_token(lex_cptr& ptr, parse_pred pred);

    std::optional<lex_cptr> test_token_val(lex_cptr &ptr, std::string_view val);
    std::optional<lex_cptr> test_token_val(lex_cptr &ptr, std::span<const std::string_view> vals);
    std::optional<lex_cptr> test_token_type(lex_cptr &ptr, lex::lex_type type);
    std::optional<lex_cptr> test_token_type(lex_cptr &ptr, std::span<const lex::lex_type> types);

    std::optional<lex_cptr> find_by_tok_val(lex_cptr start, lex_cptr end, std::string_view val);
    std::optional<lex_cptr> find_by_tok_type(lex_cptr start, lex_cptr end, lex::lex_type type);

    bool is_variable_identifier(lex_cptr token);

    loop_fn test_token_predicate(auto pred);

    std::optional<ast_node> gen_variable_identifier(lex_cptr& token);

    ast_node parse_until(lex_cptr& ptr, lex_cptr end, std::string_view until, parse_fn fn);
    ast_node parse_between(lex_cptr& ptr, parse_fn fn);
    ast_node parse_between(lex_cptr& ptr, std::string_view exp_val, parse_fn fn);

    std::optional<ast_node> try_parse(lex_cptr &ptr, const lex_cptr end, parse_fn fn);

    ast_node try_parse(lex_cptr &ptr, lex_cptr end, parse_fn fn, auto... fn_va);

    std::vector<ast_node> parse_split(lex_cptr& ptr, lex_cptr end, std::string_view split_val, parse_fn fn);
    std::vector<ast_node> capture_contiguous(lex_cptr& ptr, lex_cptr end, loop_fn fn);
}
