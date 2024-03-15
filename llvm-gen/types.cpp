#include "types.h"

#include <unordered_map>
#include <llvm/IR/Type.h>

#include "../ast/data/ast_nodes.h"

using namespace cg_llvm;

const std::unordered_map<ast::nodes::intrinsic_types, type_getter> type_generators {
    { ast::nodes::intrinsic_types::i8, AS_TYPE_GEN(llvm::Type::getInt8Ty) },
    { ast::nodes::intrinsic_types::i16, AS_TYPE_GEN(llvm::Type::getInt16Ty) },
    { ast::nodes::intrinsic_types::i32, AS_TYPE_GEN(llvm::Type::getInt32Ty) },
    { ast::nodes::intrinsic_types::i64, AS_TYPE_GEN(llvm::Type::getInt64Ty) },

    { ast::nodes::intrinsic_types::f32, AS_TYPE_GEN(llvm::Type::getFloatTy) },
    { ast::nodes::intrinsic_types::f64, AS_TYPE_GEN(llvm::Type::getDoubleTy) },

    { ast::nodes::intrinsic_types::char_, AS_TYPE_GEN(llvm::Type::getInt8Ty) },

    { ast::nodes::intrinsic_types::bool_, AS_TYPE_GEN(llvm::Type::getInt8Ty) },
    { ast::nodes::intrinsic_types::void_, AS_TYPE_GEN(llvm::Type::getVoidTy) }
};

llvm::Type* cg_llvm::get_llvm_type(const ast::nodes::value_type &val_type, llvm::LLVMContext &context) {
    const auto& type = val_type.type;
    llvm::Type* llvm_type;

    switch (type.index()) {
        using namespace ast::nodes;

        case INTRINSIC:
            llvm_type = type_generators.at(std::get<intrinsic_types>(type))(context);
            break;
        case NON_INTRINSIC:
            throw std::runtime_error("Non-intrinsic types are not supported yet.");
        default:
            std::unreachable();
    }

    return val_type.is_pointer ?
        reinterpret_cast<llvm::Type *>(llvm_type->getPointerTo()) :
        llvm_type;
}


