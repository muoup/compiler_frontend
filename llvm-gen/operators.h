#pragma once

namespace llvm {
    class Value;
}

namespace cg_llvm {
    struct scope_data;

    struct balance_result {
        llvm::Value *lhs, *rhs;
        bool is_int;
    };

    llvm::Value* generate_binop();
    llvm::Value* generate_unop();

    balance_result balance_sides(llvm::Value* lhs, llvm::Value* rhs, const scope_data& data);
}
