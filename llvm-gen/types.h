#pragma once

#include "basic_codegen.h"

namespace llvm {
    class Type;
    class LLVMContext;
}

#define AS_TYPE_GEN(x) reinterpret_cast<type_getter>(x)

using type_getter = llvm::Type *(*)(llvm::LLVMContext &);

namespace cg {
    const struct_definition* get_struct_ref(const ast::nodes::variable_type &val_type, scope_data &scope);
    llvm::Type* get_llvm_type(const ast::nodes::variable_type &val_type, scope_data &scope);
}
