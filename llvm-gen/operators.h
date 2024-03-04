#pragma once

namespace ast {
    struct ast_node;
}

namespace llvm {
    class Value;
}

namespace cg_llvm {
    struct scope_data;

    struct balance_result {
        llvm::Value *lhs, *rhs;
        bool is_int;
    };

    llvm::Value* generate_binop(const ast::ast_node &node, scope_data &data);
    llvm::Value* generate_unop(const ast::ast_node &node, scope_data &data);

    llvm::Value* generate_assignment(const ast::ast_node &node, scope_data &data);

    balance_result balance_sides(llvm::Value* lhs, llvm::Value* rhs, const scope_data& data);
}
