#pragma once
#include <llvm/IR/Type.h>
#include "../ast/data/ast_nodes.h"

namespace llvm {
    class Value;
}

namespace ast::nodes {
    struct expression;
    enum class bin_op_type;
}

namespace cg {
    struct scope_data;

    struct balance_result {
        llvm::Value *lhs, *rhs;
    };

    llvm::Instruction::BinaryOps get_llvm_binop(ast::nodes::bin_op_type type, bool is_fp);

    llvm::Value* varargs_cast(llvm::Value *val, const scope_data& scope);

    balance_result balance_sides(llvm::Value *lhs, llvm::Value *rhs, const scope_data& data);

    llvm::Value* load_if_ref(const std::unique_ptr<ast::nodes::expression> &expr, scope_data& data);

    llvm::Value* attempt_cast(llvm::Value *val, llvm::Type *to_type, const scope_data &data);

    llvm::Value* generate_bin_op(const std::unique_ptr<ast::nodes::expression> &left, const std::unique_ptr<ast::nodes::expression> &right, const ast::nodes::bin_op_type type, cg::scope_data &scope);

    llvm::Value* generate_accessor(const std::unique_ptr<ast::nodes::expression> &left, const std::unique_ptr<ast::nodes::expression> &right, cg::scope_data &scope);
}
