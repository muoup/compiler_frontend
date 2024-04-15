//
// Created by zbv11 on 15-Apr-24.
//

#ifndef LLVM_FRONTEND_DATA_H
#define LLVM_FRONTEND_DATA_H

#include <unordered_map>

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>

namespace ast::nodes {
    enum class bin_op_type;
}

namespace cg::data {

    extern const std::unordered_map<ast::nodes::bin_op_type, llvm::Instruction::BinaryOps> binop_map;
    extern const std::unordered_map<ast::nodes::bin_op_type, llvm::CmpInst::Predicate> i_cmp_map;
    extern const std::unordered_map<ast::nodes::bin_op_type, llvm::CmpInst::Predicate> f_cmp_map;

} // cg

#endif //LLVM_FRONTEND_DATA_H
