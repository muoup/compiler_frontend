#pragma once
#include <set>
#include <string>
#include <vector>

namespace lex {
    enum class naive_type {
        NONE, // No token
        KEYWORD, // if, while, for, etc.
        IDENTIFIER, // Variable name, function name, etc.
        INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, CHAR_LITERAL,
        SYMBOL, // +, -, *, /, and punctuators like (, ), {, }, etc.
    };

    struct naive_token {
        naive_type type;
        std::string_view span;
    };

    struct naive_lex_data {
        std::vector<naive_token> tokens;
    };

    const std::set<std::string_view> KEYWORD_SET = {
        "if", "while", "for", "switch",

        "i8", "i16", "i32", "i64",
        "u8", "u16", "u32", "u64",
        "f32", "f64",

        "bool", "char", "void"
    };

    const std::set<char> SYMBOL_SET = {
        '=', '+', '-', '*', '/', '%',
        '!', '&', '|', '^', '~', '<', '>', '?', ':',

        '{', '}', '(', ')', '[', ']', ';', ',', '.', '#'
    };

    const std::set<std::string_view> SPECIAL_SYMBOL = {
        "==", "!=", "<=", ">=",
        "&&", "||",
        "++", "--",
        "+=", "-=", "*=", "/=", "%=",
        "&=", "|=", "^=",
        "<<", ">>"
    };

    std::vector<naive_token> naive_lex(std::string_view code);
}
