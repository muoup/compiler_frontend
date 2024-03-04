#include "operators.h"

#include <stdexcept>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Type.h>

#include "codegen.h"
#include "../ast/declarations.h"

using namespace cg_llvm;

const std::unordered_map<std::string_view, llvm::Instruction::BinaryOps> i_binop_map {
    { "+", llvm::Instruction::BinaryOps::Add },
    { "-", llvm::Instruction::BinaryOps::Sub },
    { "*", llvm::Instruction::BinaryOps::Mul },
    { "/", llvm::Instruction::BinaryOps::SDiv },
    { "%", llvm::Instruction::BinaryOps::SRem },
    { "&", llvm::Instruction::BinaryOps::And },
    { "|", llvm::Instruction::BinaryOps::Or },
    { "^", llvm::Instruction::BinaryOps::Xor },
    { "<<", llvm::Instruction::BinaryOps::Shl },
    { ">>", llvm::Instruction::BinaryOps::AShr }
};

const std::unordered_map<std::string_view, llvm::Instruction::BinaryOps> f_binop_map {
    { "+", llvm::Instruction::BinaryOps::FAdd },
    { "-", llvm::Instruction::BinaryOps::FSub },
    { "*", llvm::Instruction::BinaryOps::FMul },
    { "/", llvm::Instruction::BinaryOps::FDiv },
    { "%", llvm::Instruction::BinaryOps::FRem },
    { "&", llvm::Instruction::BinaryOps::And },
    { "|", llvm::Instruction::BinaryOps::Or },
    { "^", llvm::Instruction::BinaryOps::Xor },
    { "<<", llvm::Instruction::BinaryOps::Shl },
    { ">>", llvm::Instruction::BinaryOps::AShr }
};

balance_result cg_llvm::balance_sides(llvm::Value* lhs, llvm::Value* rhs, const scope_data& data) {
    bool is_l_int = lhs->getType()->isIntegerTy();
    bool is_r_int = rhs->getType()->isIntegerTy();

    // If both are integers, we can just return them as is.
    if (is_l_int == is_r_int)
        return { lhs, rhs };

    return balance_result {
        .lhs = (is_l_int) ? data.builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(data.context)) : lhs,
        .rhs = (is_r_int) ? rhs : data.builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(data.context)),
    };
}

llvm::Value* cg_llvm::generate_binop(const ast::ast_node &node, scope_data &data) {
    if (node.type != ast::ast_node_type::BIN_OP)
        throw std::runtime_error("Invalid node type for binary operation generation.");
    if (node.data == "=")
        return generate_assignment(node, data);

    const auto [lhs, rhs, is_int] = balance_sides(
        generate_expression(node.children[0], data),
        generate_expression(node.children[1], data),
        data
    );
    const auto& binop_map = is_int ? i_binop_map : f_binop_map;

    if (!binop_map.contains(node.data))
        throw std::runtime_error("Invalid binary operator.");

    return data.builder.CreateBinOp(binop_map.at(node.data), lhs, rhs);
}

llvm::Value* cg_llvm::generate_assignment(const ast::ast_node& node, scope_data& data) {
    if (node.type != ast::ast_node_type::BIN_OP)
        throw std::runtime_error("Invalid node type for assignment generation.");

    const auto& lh_type = node.children[0].type;

    if (lh_type != ast::ast_node_type::INITIALIZATION && lh_type != ast::ast_node_type::VARIABLE)
        throw std::runtime_error("Invalid left-hand side type for assignment.");

    auto *lhs = generate_expression(node.children[0], data);
    auto *rhs = generate_expression(node.children[1], data);

    // Assign the value to the variable.
    return data.builder.CreateStore(rhs, lhs);
}