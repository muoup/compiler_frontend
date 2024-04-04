#pragma once
#include <unordered_map>
#include "abstract_data.h"
#include "llvm/IR/Instruction.h"

namespace ast::pm {
    const extern std::unordered_map<nodes::bin_op_type, uint16_t> binop_prec;
    const extern std::unordered_map<std::string_view, nodes::bin_op_type> binop_type_map;
    const extern std::unordered_map<std::string_view, nodes::un_op_type> unop_type_map;
    const extern std::unordered_map<std::string_view, nodes::assn_type> assign_type_map;

    const extern std::unordered_map<std::string_view, nodes::intrinsic_types> intrin_map;

    const extern std::unordered_map<ast::nodes::bin_op_type, llvm::Instruction::BinaryOps> binop_map;
}
