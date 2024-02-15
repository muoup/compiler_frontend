#pragma once
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace lex {
    enum class lex_type;
    struct lex_token;
}

namespace ast {
    using lex_cptr = std::vector<lex::lex_token>::const_iterator;
    using lex_ptr = std::vector<lex::lex_token>::iterator;

    void throw_unexpected(const lex::lex_token& token, std::string_view expected = "No explanation given.");
    void throw_unclosed(const lex::lex_token& token, std::string_view expected = "No explanation given.");

    lex_cptr assert_token_type(lex_cptr& ptr, lex::lex_type type);
    lex_cptr assert_token_type(lex_cptr& ptr, std::span<const lex::lex_type> types);
    lex_cptr assert_token_val(lex_cptr& ptr, std::string_view val);
    lex_cptr assert_token_val(lex_cptr& ptr, std::span<const std::string_view> vals);

    bool test_token_val(lex_cptr &ptr, std::string_view val);
    bool test_token_val(lex_cptr &ptr, std::span<const std::string_view> vals);
    bool test_token_type(lex_cptr &ptr, lex::lex_type type);
    bool test_token_type(lex_cptr &ptr, std::span<const lex::lex_type> types);

    std::optional<lex_ptr> find_by_tok_val(lex_ptr start, lex_ptr end, std::string_view val);
    std::optional<lex_ptr> find_by_tok_type(lex_ptr start, lex_ptr end, lex::lex_type type);
}
