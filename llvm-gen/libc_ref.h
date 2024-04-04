#pragma once
#include <unordered_map>
#include <vector>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>

#include "basic_codegen.h"
#include "types.h"

namespace cg {
    constexpr std::string_view libc_prefix = "__libc_";

    const type_getter str_ptr = [] (llvm::LLVMContext& context) {
        return static_cast<llvm::Type *>(llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0));
    };

    struct clib_func {
        bool varargs;
        type_getter return_type;
        std::vector<type_getter> args;
    };

    inline std::unordered_map<std::string_view, clib_func> func_map = {
        { "puts", { false, AS_TYPE_GEN(llvm::Type::getInt32Ty), { str_ptr } } },
        { "printf", { true, AS_TYPE_GEN(llvm::Type::getInt32Ty), { str_ptr } } },
        { "scanf", { true, AS_TYPE_GEN(llvm::Type::getInt32Ty), { str_ptr } } },
        { "malloc", { false, AS_TYPE_GEN(llvm::Type::getInt8PtrTy), { AS_TYPE_GEN(llvm::Type::getInt64Ty) } } },
        { "free", { false, AS_TYPE_GEN(llvm::Type::getVoidTy), { AS_TYPE_GEN(llvm::Type::getInt8PtrTy) } } },
        { "scanf", { true, AS_TYPE_GEN(llvm::Type::getInt32Ty), { str_ptr } } }
    };

    llvm::Function *get_libc_fn(const std::string_view name, scope_data &scope);
}
