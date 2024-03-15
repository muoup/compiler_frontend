#pragma once
#include <llvm/IR/Type.h>

namespace ast {
    namespace nodes {
        struct un_op;
        struct bin_op;
        struct assignment;
    }

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

    llvm::Value* generate_binop(const ast::nodes::bin_op &node, scope_data &data);
    llvm::Value* generate_unop(const ast::nodes::un_op &un_op, scope_data &data);

    llvm::Value* generate_assignment(const ast::nodes::assignment &node, scope_data &data);

    balance_result balance_sides(llvm::Value *lhs, llvm::Value *rhs, const scope_data& data);

    llvm::Value* attempt_cast(llvm::Value *val, llvm::Type *to_type, const scope_data &data);
}
