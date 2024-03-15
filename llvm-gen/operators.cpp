#include "operators.h"

#include <stdexcept>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Type.h>

#include "codegen.h"
#include "../ast/declarations.h"
#include "../ast/data/ast_nodes.h"

using namespace cg_llvm;

const std::unordered_map<ast::nodes::bin_op_type, llvm::Instruction::BinaryOps> binop_map {
    { ast::nodes::bin_op_type::add, llvm::Instruction::BinaryOps::Add },
    { ast::nodes::bin_op_type::sub, llvm::Instruction::BinaryOps::Sub },
    { ast::nodes::bin_op_type::mul, llvm::Instruction::BinaryOps::Mul },
    { ast::nodes::bin_op_type::div, llvm::Instruction::BinaryOps::SDiv },
    { ast::nodes::bin_op_type::mod, llvm::Instruction::BinaryOps::SRem },
    { ast::nodes::bin_op_type::and_, llvm::Instruction::BinaryOps::And },
    { ast::nodes::bin_op_type::or_, llvm::Instruction::BinaryOps::Or },
    { ast::nodes::bin_op_type::xor_, llvm::Instruction::BinaryOps::Xor },
    { ast::nodes::bin_op_type::shl, llvm::Instruction::BinaryOps::Shl },
    { ast::nodes::bin_op_type::shr, llvm::Instruction::BinaryOps::AShr },
};

balance_result cg_llvm::balance_sides(llvm::Value* lhs, llvm::Value* rhs, const scope_data& data) {
    const bool is_l_int = lhs->getType()->isIntegerTy();
    const bool is_r_int = rhs->getType()->isIntegerTy();

    // If both are integers, we can just return them as is.
    if (is_l_int == is_r_int)
        return balance_result {
            lhs, rhs
        };

    return balance_result {
        .lhs = is_l_int ? data.builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(data.context)) : lhs,
        .rhs = is_r_int ? rhs : data.builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(data.context)),
    };
}

llvm::Value * cg_llvm::attempt_cast(llvm::Value *val, llvm::Type *to_type, const scope_data &data) {
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

llvm::Value* cg_llvm::generate_binop(const ast::nodes::bin_op &node, scope_data &data) {
    const auto [lhs, rhs, is_int] = balance_sides(
        generate_expression(*node.left, data),
        generate_expression(*node.right, data),
        data
    );
    const auto binop_type =
        static_cast<llvm::Instruction::BinaryOps>(binop_map.at(node.type) + is_int);
    return data.builder.CreateBinOp(binop_type, lhs, rhs);
}

llvm::Value* cg_llvm::generate_unop(const ast::nodes::un_op &un_op, scope_data &data) {
    auto *val = generate_expression(*un_op.value, data);
    const auto &expr = *un_op.value;

    switch (un_op.type) {
        using namespace ast::nodes;
        case un_op_type::l_not:
            return data.builder.CreateNot(val);
        case un_op_type::dereference:
            if (!val->getType()->isPointerTy())
                throw std::runtime_error("Dereferencing non-pointer type");

            return data.builder.CreateLoad(val->getType(), val);
        case un_op_type::address_of:
            if (expr.value.index() != VARIABLE)
                throw std::runtime_error("Cannot take address of non-variable");

            return generate_variable(std::get<variable>(expr.value), data);
        case un_op_type::bit_not:
            if (!val->getType()->isIntegerTy())
                throw std::runtime_error("Bitwise not on non-integer type");

            return data.builder.CreateBinOp(
                llvm::Instruction::BinaryOps::Xor,
                val, llvm::ConstantInt::get(val->getType(), -1)
            );
        case un_op_type::negate:
            if (val->getType()->isIntegerTy())
                return data.builder.CreateNeg(val);
            else if (val->getType()->isFloatingPointTy())
                return data.builder.CreateFNeg(val);
            else
                throw std::runtime_error("Negation on non-numeric type");
        default:
            throw std::runtime_error("Invalid unary operator");
    }
}

llvm::Value* cg_llvm::generate_assignment(const ast::nodes::assignment& node, scope_data& data) {
    llvm::Value* lhs;

    switch (node.variable.index()) {
        using namespace ast::nodes;
        case ASSIGN_INITIALIZATION:
            lhs = generate_initialization(std::get<initialization>(node.variable), data);
            break;
        case ASSIGN_VARIABLE:
            lhs = generate_variable(std::get<variable>(node.variable), data);
            break;
        default:
            throw std::runtime_error("Invalid assignment type");
    }

    auto *rhs = generate_expression(*node.value, data);

    // Assign the value to the variable.
    return data.builder.CreateStore(rhs, lhs);
}