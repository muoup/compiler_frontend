#include "operators.h"

#include <stdexcept>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Type.h>

#include "basic_codegen.h"
#include "types.h"

using namespace cg;
using namespace ast;

struct pseudo_bin_op {
    const std::unique_ptr<nodes::expression> &left, &right;
    nodes::bin_op_type type;
};

llvm::Value * cg::attempt_cast(llvm::Value *val, llvm::Type *to_type, const scope_data &data) {
    const auto *from_type = val->getType();

    if (from_type == to_type)
        return val;

    if (from_type->isIntegerTy() && to_type->isIntegerTy())
        return data.builder.CreateIntCast(val, to_type, true);
    else if (from_type->isFloatingPointTy() && to_type->isFloatingPointTy())
        return data.builder.CreateFPCast(val, to_type);
    else if (from_type->isIntegerTy() && to_type->isFloatingPointTy())
        return data.builder.CreateSIToFP(val, to_type);
    else if (from_type->isFloatingPointTy() && to_type->isIntegerTy())
        return data.builder.CreateFPToSI(val, to_type);

    if (from_type->isPointerTy() && to_type->isPointerTy())
        return data.builder.CreatePointerCast(val, to_type);

    throw std::runtime_error("Invalid cast.");
}

llvm::Value* cg::varargs_cast(llvm::Value *val, const scope_data &scope) {
    if (val->getType()->isIntegerTy())
        return attempt_cast(val, llvm::Type::getInt32Ty(scope.context), scope);

    if (val->getType()->isPointerTy())
        return val;

    throw std::runtime_error("For now, varargs parameters are limited to i32 and pointers.");
}

llvm::Value *generate_comparison(const pseudo_bin_op &ref, cg::scope_data &scope) {
    auto lhs = ref.left->generate_code(scope);
    auto rhs = ref.right->generate_code(scope);

    if (ref.left->get_type().is_fp()) {
        return scope.builder.CreateFCmp(
                *pm::find_element(pm::f_cmp_map, ref.type),
                lhs, rhs
        );
    } else {
        auto i_cmp = *pm::find_element(pm::i_cmp_map, ref.type);

        if (ref.type == nodes::bin_op_type::eq || ref.type == nodes::bin_op_type::neq)
            return scope.builder.CreateICmp(i_cmp, lhs, rhs);

        if (ref.left->get_type().is_signed())
            i_cmp = static_cast<llvm::CmpInst::Predicate>(i_cmp + 4);

        return scope.builder.CreateICmp(
                i_cmp,
                lhs, rhs
        );
    }
}

llvm::Instruction::BinaryOps cg::get_llvm_binop(const nodes::bin_op_type type, const bool is_fp) {
    auto mapped_type = pm::basic_binop_map.at(type);

    return static_cast<llvm::Instruction::BinaryOps>(mapped_type + is_fp);
}

llvm::Value* generate_basic_op(const pseudo_bin_op &ref, cg::scope_data &scope) {
    auto *lhs = ref.left->generate_code(scope);
    auto *rhs = ref.right->generate_code(scope);

    return scope.builder.CreateBinOp(
            get_llvm_binop(ref.type, lhs->getType()->isFloatingPointTy()),
            lhs, rhs);
}

llvm::Value* cg::generate_accessor(const std::unique_ptr<ast::nodes::expression> &left, const std::unique_ptr<ast::nodes::expression> &right, cg::scope_data &scope) {
    auto l_code = left->generate_code(scope);
    auto r_code = right->generate_code(scope);

    auto l_type = left->get_type();
    auto r_type = right->get_type();

    if (l_type.is_pointer()) {
        return scope.builder.CreateGEP(
            get_llvm_type(left->get_type().type, scope),
            l_code,
            r_code
        );
    }

    const nodes::var_ref* get_var = dynamic_cast<nodes::var_ref*>(right.get());

    if (!get_var)
        throw std::runtime_error("Accessing non-member of a struct");

    if (l_type.is_intrinsic())
        throw std::runtime_error("Cannot access member of intrinsic type!");

    auto struct_name = std::get<std::string_view>(l_type.type);
    auto &struct_decl = scope.get_struct(struct_name);

    auto field_index = std::ranges::find_if(struct_decl.field_decls, [get_var] (const auto &field) {
        return field.var_name == get_var->var_name;
    });

    if (field_index == struct_decl.field_decls.end())
        throw std::runtime_error("Invalid right side of accessor.");

    auto field_index_int = std::distance(struct_decl.field_decls.begin(), field_index);

    return scope.builder.CreateStructGEP(struct_decl.struct_type, l_code, field_index_int);
}

llvm::Value* cg::generate_bin_op(const std::unique_ptr<ast::nodes::expression> &left, const std::unique_ptr<ast::nodes::expression> &right, const ast::nodes::bin_op_type type, cg::scope_data &scope) {
    const pseudo_bin_op ref { left, right, type };

    if (pm::basic_binop_map.contains(type))
        return generate_basic_op(ref, scope);

    if (pm::i_cmp_map.contains(type) || pm::f_cmp_map.contains(type))
        return generate_comparison(ref, scope);

    if (type == nodes::bin_op_type::acc)
        return generate_accessor(left, right, scope);

    throw std::runtime_error("Invalid binary operator.");
}