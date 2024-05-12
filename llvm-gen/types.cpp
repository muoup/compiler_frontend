#include "types.h"

#include <unordered_map>
#include <llvm/IR/Type.h>

#include "../ast/data/node_interfaces.h"

using namespace cg;

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

const struct_definition* cg::get_struct_ref(const ast::nodes::variable_type &val_type, scope_data &scope) {
    const auto& type = val_type.type;

    if (std::holds_alternative<ast::nodes::intrinsic_types>(type)) {
        return nullptr;
    }

    // Non-intrinsic type
    std::string_view type_name = std::get<std::string_view>(type);
    return &scope.get_struct(type_name);
}

llvm::Type* cg::get_llvm_type(const ast::nodes::variable_type &val_type, scope_data &scope) {
    const auto& type = val_type.type;
    llvm::Type* derived_type = std::holds_alternative<ast::nodes::intrinsic_types>(type) ?
            type_generators.at(std::get<ast::nodes::intrinsic_types>(type))(scope.context) :
            scope.get_struct(std::get<std::string_view>(type)).struct_type;

    if (val_type.pointer_depth == 0)
        return derived_type;

    return llvm::PointerType::get(derived_type, val_type.pointer_depth);
}