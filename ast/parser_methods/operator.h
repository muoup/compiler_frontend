#pragma once
#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace ast::pm {
    const std::unordered_map<std::string_view, uint16_t> binop_prec {
        {"=", 1},
        {"<", 10},
        {">", 10},
        {"+", 20},
        {"-", 20},
        {"*", 40},
        {"/", 40},
        {"%", 40},
    };
}
