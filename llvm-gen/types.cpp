#include "types.h"

#include <unordered_map>
#include <llvm/IR/Type.h>

#include "../ast/declarations.h"

using namespace cg_llvm;

#define AS_TYPE_GEN(x) reinterpret_cast<type_getter>(llvm::Type::x)

using type_getter = llvm::Type *(*)(llvm::LLVMContext &);

const std::unordered_map<std::string_view, type_getter> type_generators {
    { "i8", AS_TYPE_GEN(getInt8Ty) },
    { "i16", AS_TYPE_GEN(getInt16Ty) },
    { "i32", AS_TYPE_GEN(getInt32Ty) },
    { "i64", AS_TYPE_GEN(getInt64Ty) },

    { "f32", AS_TYPE_GEN(getFloatTy) },
    { "f64", AS_TYPE_GEN(getDoubleTy) },

    { "char", AS_TYPE_GEN(getInt8Ty) },

    { "bool", AS_TYPE_GEN(getInt8Ty) },
    { "void", AS_TYPE_GEN(getVoidTy) }
};

llvm::Type* cg_llvm::get_llvm_type(const ast::ast_node &node, llvm::LLVMContext &context) {
    return type_generators.at(node.metadata)(context);
}


