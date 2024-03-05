#include "libc_ref.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

#include "codegen.h"

using namespace cg_llvm;

llvm::Function *cg_llvm::get_libc_fn(const std::string_view name, scope_data& scope) {
    std::vector<llvm::Type*> types;

    const auto proper_name = name.substr(libc_prefix.size());

    const auto& func = func_map.at(proper_name);
    const auto& ret_type = func.return_type(scope.context);

    for (const auto& type : func.args) {
        types.push_back(type(scope.context));
    }
    const auto fn_type = llvm::FunctionType::get(ret_type, types, func.varargs);

    return static_cast<llvm::Function *>(scope.root->getOrInsertFunction(proper_name, fn_type).getCallee());
}