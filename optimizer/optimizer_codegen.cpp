#include "optimizer_nodes.h"
#include "../llvm-gen/basic_codegen.h"

llvm::Value *opt::nodes::tail_call::generate_code(cg::scope_data &scope) const {
    auto* call = (llvm::CallInst*) ast::nodes::method_call::generate_code(scope);

    call->setTailCall();

    return call;
}