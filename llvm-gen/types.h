#pragma once

namespace llvm {
    class Type;
    class LLVMContext;
}

namespace ast {
    struct ast_node;
}

namespace cg_llvm {
    llvm::Type* get_llvm_type(const ast::ast_node &node, llvm::LLVMContext &context);
}
