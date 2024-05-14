#pragma once

#include <unordered_map>

#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instruction.h>

namespace ast::nodes {
    enum class bin_op_type;
}

namespace cg {

    extern const std::unordered_map<ast::nodes::bin_op_type, llvm::Instruction::BinaryOps> binop_map;
    extern const std::unordered_map<ast::nodes::bin_op_type, llvm::CmpInst::Predicate> i_cmp_map;
    extern const std::unordered_map<ast::nodes::bin_op_type, llvm::CmpInst::Predicate> f_cmp_map;

}