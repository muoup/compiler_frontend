#pragma once

namespace llvm {
    class Type;
    class LLVMContext;
}

namespace ast::nodes {
    struct value_type;
    struct type_instance;
}

#define AS_TYPE_GEN(x) reinterpret_cast<type_getter>(x)

using type_getter = llvm::Type *(*)(llvm::LLVMContext &);

namespace cg_llvm {
    llvm::Type* get_llvm_type(const ast::nodes::value_type &val_type, llvm::LLVMContext &context);
}
