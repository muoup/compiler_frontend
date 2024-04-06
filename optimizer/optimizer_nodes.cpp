#include "optimizer_nodes.h"
#include <iostream>

void opt::nodes::tail_call::print(size_t depth) const {
    std::cout << std::string(depth, '\t') << "Tail Call" << '\n';
    for (const auto& arg : arguments)
        arg->print(depth + 1);
}
