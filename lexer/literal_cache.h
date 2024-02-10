#pragma once
#include <string>
#include <unordered_map>

namespace lex {
    struct literal_cache {
        std::unordered_map<std::string, std::string> str_lit_cache;
    };
}
