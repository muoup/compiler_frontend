#pragma once
#include <llvm/IR/Type.h>
#include "../ast/data/ast_nodes.h"

namespace llvm {
    class Value;
}

namespace cg {
    struct scope_data;

    struct balance_result {
        llvm::Value *lhs, *rhs;
        bool is_int;
    };

    llvm::Value* varargs_cast(llvm::Value *val, const scope_data& scope);

    balance_result balance_sides(llvm::Value *lhs, llvm::Value *rhs, const scope_data& data);

    llvm::Value* attempt_cast(llvm::Value *val, llvm::Type *to_type, const scope_data &data);

    llvm::Value* generate_binop(const ast::nodes::bin_op &binop, cg::scope_data &scope);
}
