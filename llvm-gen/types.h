#pragma once

namespace llvm {
    class Type;
    class LLVMContext;
}

namespace ast {
    struct ast_node;
}

#define AS_TYPE_GEN(x) reinterpret_cast<type_getter>(x)

using type_getter = llvm::Type *(*)(llvm::LLVMContext &);

namespace cg_llvm {
    llvm::Type* get_llvm_type(const ast::ast_node &node, llvm::LLVMContext &context);
}
