#pragma once

#include <string_view>
#include <optional>
#include <unordered_set>
#include <vector>

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
        EXPR_SYMBOL, // +, -, *, /, etc
        ASSN_SYMBOL, // =, +=, -=, etc
        PUNCTUATOR
    };

    struct lex_token {
        lex_type type;
        std::string_view span;

        std::optional<lex_ptr> closer;
    };

    inline std::unordered_set<std::string_view> KEYWORD_SET {
        "if", "while", "for", "switch",

        "mut",

        "return"
    };

    inline std::unordered_set<std::string_view> PRIMITIVES_SET {
        "i8", "i16", "i32", "i64",
        "u8", "u16", "u32", "u64",
        "f32", "f64",

        "bool", "char", "void",
    };

    inline std::unordered_set<std::string_view> EXPR_SYMBOL {
        "+", "-", "*", "/", "%",
        "!", "&", "|", "^", "~", "<", ">", "?", ":",
        "<<", ">>"

        ",", ".", "#", ";",

        "==", "!=", "<=", ">=", "&&", "||",
        "++", "--", "**"
    };

    inline std::unordered_set<std::string_view> ASSN_SYMBOL = {
        "=", "+=", "-=", "*=", "/=", "%=",
        "&=", "|=", "^=",
    };

    inline std::unordered_set<char> PUNCTUATOR_SET = {
        '{', '}', '(', ')', '[', ']'
    };

    std::vector<lex_token> lex(std::string_view code);
}
