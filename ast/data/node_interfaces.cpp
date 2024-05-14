#include "node_interfaces.h"
#include <stack>
#include <iostream>

using namespace ast;

void nodes::printable::print(const size_t depth) const {
    std::cout << std::string(depth * 2, ' ') << node_name();
    if (has_print_details()) {
        std::cout << " ( ";
        print_details();
        std::cout << ")";
    }
    std::cout << '\n';

    for (const auto &node : children().nodes) {
        node->print(depth + 1);
    }
}