#include "operators.h"

#include <stdexcept>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Type.h>

#include "basic_codegen.h"
#include "../ast/data/data_maps.h"

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
                llvm::CmpInst::Predicate::FCMP_OEQ,
                lhs, rhs
        );
    } else {
        return scope.builder.CreateICmp(
                llvm::CmpInst::Predicate::ICMP_EQ,
                lhs, rhs
        );
    }
}

llvm::Value* generate_basic_op(const pseudo_bin_op &ref, cg::scope_data &scope) {
    auto *lhs = ref.left->generate_code(scope);
    auto *rhs = ref.right->generate_code(scope);

    auto mapped_type = pm::basic_binop_map.at(ref.type);

    const auto bin_op_type = static_cast<llvm::Instruction::BinaryOps>(mapped_type + rhs->getType()->isFloatingPointTy());

    return scope.builder.CreateBinOp(bin_op_type, lhs, rhs);
}

llvm::Value* cg::generate_accessor(const std::unique_ptr<ast::nodes::expression> &left, const std::unique_ptr<ast::nodes::expression> &right, cg::scope_data &scope) {
    auto *l_ref = dynamic_cast<const nodes::var_ref*>(left.get());
    auto *r_ref = dynamic_cast<const nodes::var_ref*>(right.get());

    if (l_ref == nullptr)
        throw std::runtime_error("Invalid left side of accessor.");

    auto get_var = scope.get_variable(l_ref->var_name);
    auto var_alloc = get_var.var_allocation;
    llvm::Value *var_val = (llvm::Value*) get_var.var_allocation;

    if (get_var.struct_type == nullptr)
        throw std::runtime_error("Invalid left side of accessor.");

    auto *struct_type = get_var.struct_type->struct_type;
    auto field_index = std::ranges::find(get_var.struct_type->field_names, r_ref->var_name);

    if (field_index == get_var.struct_type->field_names.end())
        throw std::runtime_error("Invalid right side of accessor.");

    auto field_index_int = std::distance(get_var.struct_type->field_names.begin(), field_index);

    return scope.builder.CreateStructGEP(struct_type, var_val, field_index_int);
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