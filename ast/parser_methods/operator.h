#pragma once
#include <cstdint>
#include <functional>
#include <string_view>
#include <unordered_map>

namespace ast::pm {
    using binop_fn = std::function<int(int, int)>;

    const std::unordered_map<std::string_view, uint16_t> binop_prec {
        {"<", 10},
        {">", 10},
        {"+", 20},
        {"-", 20},
        {"*", 40},
        {"/", 40},
        {"%", 40},
        {"**", 80}
    };

    const std::unordered_map<std::string_view, binop_fn> binop_fn_map {
        {"<", [](const int a, const int b) { return a < b; }},
        {">", [](const int a, const int b) { return a > b; }},
        {"+", [](const int a, const int b) { return a + b; }},
        {"-", [](const int a, const int b) { return a - b; }},
        {"*", [](const int a, const int b) { return a * b; }},
        {"/", [](const int a, const int b) { return a / b; }},
        {"%", [](const int a, const int b) { return a % b; }},
        {"**", [](const int a, const int b) { return pow(a, b); }}
    };

    static bool greater_prec(const std::string_view op1, const std::string_view op2) {
        return binop_prec.at(op1) > binop_prec.at(op2);
    }
}
