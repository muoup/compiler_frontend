#include "declarations.h"

#include <algorithm>
#include <string_view>
#include <vector>

using namespace ast;

ast_node& ast_node::add_child(ast_node&& child) {
    this->children.emplace_back(child);
    return children.back();
}

ast_node& ast_node::add_child(const ast_node_type type, const std::string_view metadata, const std::string_view data) {
    return this->add_child (ast_node { type, metadata, data });
}

template<typename... Args>
void ast_node::add_children(ast_node&& child, Args&&... children) {
    add_child(std::move(child));

    if constexpr (sizeof...(children) > 0)
        add_children((std::move(children), ...));
}

void ast_node::add_children(std::vector<ast_node>&& children) {
    for (auto&& child : children)
        this->add_child(std::move(child));
}

ast_node ast_node::with_data(const std::string_view data) {
    this->data = data;
    return *this;
}

ast_node ast_node::with_metadata(const std::string_view metadata) {
    this->metadata = metadata;
    return *this;
}