#include "types.h"

#include <unordered_map>
#include <llvm/IR/Type.h>

#include "../ast/data/node_interfaces.h"

using namespace cg;

const std::unordered_map<ast::nodes::intrinsic_type, type_getter> type_generators {
    { ast::nodes::intrinsic_type::i8,    AS_TYPE_GEN(llvm::Type::getInt8Ty) },
    { ast::nodes::intrinsic_type::i16,   AS_TYPE_GEN(llvm::Type::getInt16Ty) },
    { ast::nodes::intrinsic_type::i32,   AS_TYPE_GEN(llvm::Type::getInt32Ty) },
    { ast::nodes::intrinsic_type::i64,   AS_TYPE_GEN(llvm::Type::getInt64Ty) },

    { ast::nodes::intrinsic_type::u8,    AS_TYPE_GEN(llvm::Type::getInt8Ty)  },
    { ast::nodes::intrinsic_type::u16,   AS_TYPE_GEN(llvm::Type::getInt16Ty) },
    { ast::nodes::intrinsic_type::u32,   AS_TYPE_GEN(llvm::Type::getInt32Ty) },
    { ast::nodes::intrinsic_type::u64,   AS_TYPE_GEN(llvm::Type::getInt64Ty) },

    { ast::nodes::intrinsic_type::f32,   AS_TYPE_GEN(llvm::Type::getFloatTy) },
    { ast::nodes::intrinsic_type::f64,   AS_TYPE_GEN(llvm::Type::getDoubleTy) },

    { ast::nodes::intrinsic_type::char_, AS_TYPE_GEN(llvm::Type::getInt8Ty) },

    { ast::nodes::intrinsic_type::bool_, AS_TYPE_GEN(llvm::Type::getInt8Ty) },
    { ast::nodes::intrinsic_type::void_, AS_TYPE_GEN(llvm::Type::getVoidTy) }
};

const struct_definition* cg::get_struct_ref(const ast::nodes::variable_type &val_type, scope_data &scope) {
    const auto& type = val_type.type;

    if (std::holds_alternative<ast::nodes::intrinsic_type>(type)) {
        return nullptr;
    }

    // Non-intrinsic type
    std::string_view type_name = std::get<std::string_view>(type);
    return &scope.get_struct(type_name);
}

llvm::Type* cg::get_llvm_type(const ast::nodes::variable_type &val_type, scope_data &scope) {
    const auto& type = val_type.type;

    // Pointer type
    if (val_type.pointer_depth > 0)
        return llvm::PointerType::getUnqual(scope.context);

    // Intrinsic type
    if (std::holds_alternative<ast::nodes::intrinsic_type>(type))
        return type_generators.at(std::get<ast::nodes::intrinsic_type>(type))(scope.context);

    // Non-intrinsic / Struct type
    return scope.get_struct(std::get<std::string_view>(type)).struct_type;
}