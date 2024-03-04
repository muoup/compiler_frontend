#pragma once
#include <unordered_map>
#include <vector>
#include <llvm/IR/DerivedTypes.h>

#include "types.h"

namespace cg_llvm {
    const type_getter str_ptr = [] (llvm::LLVMContext& context) {
        return static_cast<llvm::Type *>(llvm::PointerType::get(llvm::Type::getInt8Ty(context), 0));
    };

    struct clib_func {
        bool varargs;
        type_getter return_type;
        std::vector<type_getter> args;
    };

    inline std::unordered_map<std::string_view, clib_func> func_map = {
        { "printf", { true, AS_TYPE_GEN(llvm::Type::getInt32Ty), { str_ptr } } },
    };

    llvm::FunctionType *get_fn_types(std::string_view name, llvm::LLVMContext &context);
}
