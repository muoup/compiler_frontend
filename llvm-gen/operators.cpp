#include "operators.h"

#include <stdexcept>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Type.h>

#include "basic_codegen.h"
#include "../ast/data/data_maps.h"

using namespace cg;

balance_result cg::balance_sides(llvm::Value* lhs, llvm::Value* rhs, const scope_data& data) {
    const bool is_l_int = lhs->getType()->isIntegerTy();
    const bool is_r_int = rhs->getType()->isIntegerTy();

    if (is_l_int == is_r_int)
        return balance_result { lhs, rhs };

    return balance_result {
        .lhs = is_l_int ? data.builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(data.context)) : lhs,
        .rhs = is_r_int ? rhs : data.builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(data.context)),
    };
}

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

llvm::Value* cg::generate_binop(llvm::Value *lhs, llvm::Value *rhs,
                                ast::nodes::bin_op_type type,
                                cg::scope_data &scope) {
    auto *rhs_casted = attempt_cast(rhs, lhs->getType(), scope);
    const bool is_fp = lhs->getType()->isFloatingPointTy();

    if (auto it = ast::pm::binop_map.find(type); it != ast::pm::binop_map.end()) {
        const auto bin_op_type = static_cast<llvm::Instruction::BinaryOps>(it->second + is_fp);

        return scope.builder.CreateBinOp(bin_op_type, lhs, rhs_casted);
    }

    // TODO: Unsigned Comparisons
    llvm::CmpInst::Predicate pred;

    if (is_fp) {
        pred = ast::pm::f_cmp_map.at(type);
    } else {
        pred = ast::pm::i_cmp_map.at(type);
    }

    return is_fp ?
           scope.builder.CreateFCmp(
            pred,
            lhs, rhs_casted
        ) :
           scope.builder.CreateICmp(
            pred,
            lhs, rhs_casted
        );
}