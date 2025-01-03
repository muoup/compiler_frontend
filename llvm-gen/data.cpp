#include "data.h"

#include "../ast/data/abstract_data.h"

#include <llvm/IR/Instruction.h>
#include <format>

const std::unordered_map<ast::nodes::bin_op_type, llvm::Instruction::BinaryOps> cg::binop_map {
        { ast::nodes::bin_op_type::add, llvm::Instruction::BinaryOps::Add },
        { ast::nodes::bin_op_type::sub, llvm::Instruction::BinaryOps::Sub },
        { ast::nodes::bin_op_type::mul, llvm::Instruction::BinaryOps::Mul },
        { ast::nodes::bin_op_type::div, llvm::Instruction::BinaryOps::SDiv },
        { ast::nodes::bin_op_type::mod, llvm::Instruction::BinaryOps::SRem },
        { ast::nodes::bin_op_type::b_and, llvm::Instruction::BinaryOps::And },
        { ast::nodes::bin_op_type::b_or, llvm::Instruction::BinaryOps::Or },
        { ast::nodes::bin_op_type::b_xor, llvm::Instruction::BinaryOps::Xor },
        { ast::nodes::bin_op_type::shl, llvm::Instruction::BinaryOps::Shl },
        { ast::nodes::bin_op_type::shr, llvm::Instruction::BinaryOps::AShr },
};

const std::unordered_map<ast::nodes::bin_op_type, llvm::CmpInst::Predicate> cg::i_cmp_map {
        { ast::nodes::bin_op_type::eq, llvm::CmpInst::Predicate::ICMP_EQ },
        { ast::nodes::bin_op_type::neq, llvm::CmpInst::Predicate::ICMP_NE },
        { ast::nodes::bin_op_type::lt, llvm::CmpInst::Predicate::ICMP_ULT },
        { ast::nodes::bin_op_type::gt, llvm::CmpInst::Predicate::ICMP_UGT },
        { ast::nodes::bin_op_type::lte, llvm::CmpInst::Predicate::ICMP_ULE },
        { ast::nodes::bin_op_type::gte, llvm::CmpInst::Predicate::ICMP_UGE },
};

const std::unordered_map<ast::nodes::bin_op_type, llvm::CmpInst::Predicate> cg::f_cmp_map {
        { ast::nodes::bin_op_type::eq, llvm::CmpInst::Predicate::FCMP_OEQ },
        { ast::nodes::bin_op_type::neq, llvm::CmpInst::Predicate::FCMP_ONE },
        { ast::nodes::bin_op_type::lt, llvm::CmpInst::Predicate::FCMP_OLT },
        { ast::nodes::bin_op_type::gt, llvm::CmpInst::Predicate::FCMP_OGT },
        { ast::nodes::bin_op_type::lte, llvm::CmpInst::Predicate::FCMP_OLE },
        { ast::nodes::bin_op_type::gte, llvm::CmpInst::Predicate::FCMP_OGE },
};

char escape_char(const char c) {
    switch (c) {
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'r':
            return '\r';
        case '0':
            return '\0';
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'v':
            return '\v';
        case '\\':
            return '\\';
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        default:
            throw std::runtime_error(std::format("Invalid escape sequence: \\{}", c));
    }
}

std::string cg::stringify(std::string_view str) {
    std::string out;

    out.reserve(str.size());

    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '\\') {
            if (i + 1 >= str.size())
                throw std::runtime_error("Invalid escape sequence");

            out.push_back(escape_char(str[++i]));
        } else {
            out.push_back(str[i]);
        }
    }

    return out;
}
