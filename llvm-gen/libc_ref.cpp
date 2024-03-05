#include "libc_ref.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

using namespace cg_llvm;

llvm::FunctionType *cg_llvm::get_fn_types(const std::string_view name, llvm::LLVMContext &context) {
    std::vector<llvm::Type*> types;
    const auto& func = func_map.at(name);
    const auto& ret_type = func.return_type(context);
    for (const auto& type : func.args) {
        types.push_back(type(context));
    }
    return llvm::FunctionType::get(ret_type, types, func.varargs);
}