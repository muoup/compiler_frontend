#pragma once

#include "../ast/data/ast_nodes.h"

namespace opt::nodes {
    using namespace ast::nodes;

    struct tail_call : method_call {
        std::vector<std::unique_ptr<expression>> arguments;

        ~tail_call() override = default;
        void print(size_t depth) const override;
        llvm::Value* generate_code(cg::scope_data &scope) const override;
    };
}
