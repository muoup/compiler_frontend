#pragma once

#include <string_view>
#include <optional>
#include <unordered_set>
#include <vector>

namespace ast {
    enum class ast_node_type;
}

namespace lex {
    struct lex_token;
    using str_ptr = std::string_view::const_iterator;
    using lex_ptr = std::vector<lex_token>::iterator;

    enum class lex_type {
        NONE, // No token
        KEYWORD, // if, while, for, etc.
        PRIMITIVE, // i8, i16, i32, etc.
        IDENTIFIER, // Variable name, function name, etc.
        INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, CHAR_LITERAL,
        SYMBOL, // +, -, *, /, etc
        PUNCTUATOR
    };

    struct lex_token {
        lex_type type;
        std::string_view span;

        std::optional<lex_ptr> closer;
    };

    const std::unordered_set<lex_type> LITERAL_SET {
        lex_type::INT_LITERAL, lex_type::FLOAT_LITERAL,
        lex_type::STRING_LITERAL, lex_type::CHAR_LITERAL
    };

    const std::unordered_set<std::string_view> KEYWORD_SET {
        "if", "while", "for", "switch",

        "mut",

        "return"
    };

    const std::unordered_set<std::string_view> PRIMITIVES_SET {
        "i8", "i16", "i32", "i64",
        "u8", "u16", "u32", "u64",
        "f32", "f64",

        "bool", "char", "void",
    };

    const std::unordered_set<char> SYMBOL_SET {
        '=', '+', '-', '*', '/', '%',
        '!', '&', '|', '^', '~', '<', '>', '?', ':',

        ',', '.', '#', ';'
    };

    const std::unordered_set<char> PUNCTUATOR_SET = {
        '{', '}', '(', ')', '[', ']'
    };

    const std::unordered_set<std::string_view> SPECIAL_SYMBOL = {
        "==", "!=", "<=", ">=",
        "&&", "||",
        "++", "--", "**",
        "+=", "-=", "*=", "/=", "%=",
        "&=", "|=", "^=",
        "<<", ">>"
    };

    std::vector<lex_token> lex(std::string_view code);
}
