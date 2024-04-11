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
    };

    llvm::Value* varargs_cast(llvm::Value *val, const scope_data& scope);

    balance_result balance_sides(llvm::Value *lhs, llvm::Value *rhs, const scope_data& data);

    llvm::Value* attempt_cast(llvm::Value *val, llvm::Type *to_type, const scope_data &data);

    llvm::Value* generate_binop(llvm::Value *left,
                                llvm::Value *ight,
                                ast::nodes::bin_op_type type,
                                cg::scope_data &scope);
}
