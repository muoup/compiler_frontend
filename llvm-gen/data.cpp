//
// Created by zbv11 on 15-Apr-24.
//

#include "data.h"
#include "../ast/data/abstract_data.h"
#include <llvm/IR/Instruction.h>

using enum ast::nodes::bin_op_type;

const std::unordered_map<ast::nodes::bin_op_type, llvm::Instruction::BinaryOps> cg::binop_map {
        { add, llvm::Instruction::BinaryOps::Add },
        { sub, llvm::Instruction::BinaryOps::Sub },
        { mul, llvm::Instruction::BinaryOps::Mul },
        { div, llvm::Instruction::BinaryOps::SDiv },
        { mod, llvm::Instruction::BinaryOps::SRem },
        { b_and, llvm::Instruction::BinaryOps::And },
        { b_or, llvm::Instruction::BinaryOps::Or },
        { b_xor, llvm::Instruction::BinaryOps::Xor },
        { shl, llvm::Instruction::BinaryOps::Shl },
        { shr, llvm::Instruction::BinaryOps::AShr },
};

const std::unordered_map<ast::nodes::bin_op_type, llvm::CmpInst::Predicate> cg::i_cmp_map {
        { eq, llvm::CmpInst::Predicate::ICMP_EQ },
        { neq, llvm::CmpInst::Predicate::ICMP_NE },
        { lt, llvm::CmpInst::Predicate::ICMP_ULT },
        { gt, llvm::CmpInst::Predicate::ICMP_UGT },
        { lte, llvm::CmpInst::Predicate::ICMP_ULE },
        { gte, llvm::CmpInst::Predicate::ICMP_UGE },
};

const std::unordered_map<ast::nodes::bin_op_type, llvm::CmpInst::Predicate> cg::f_cmp_map {
        { eq, llvm::CmpInst::Predicate::FCMP_OEQ },
        { neq, llvm::CmpInst::Predicate::FCMP_ONE },
        { lt, llvm::CmpInst::Predicate::FCMP_OLT },
        { gt, llvm::CmpInst::Predicate::FCMP_OGT },
        { lte, llvm::CmpInst::Predicate::FCMP_OLE },
        { gte, llvm::CmpInst::Predicate::FCMP_OGE },
};
