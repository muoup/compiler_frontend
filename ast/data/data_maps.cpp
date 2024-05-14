#include <llvm/IR/Instruction.h>
#include "data_maps.h"

using namespace ast;
using namespace nodes;

const std::unordered_map<bin_op_type, uint16_t> pm::binop_prec {
    { bin_op_type::acc,   1 },
    { bin_op_type::l_or,  2  },
    { bin_op_type::l_and, 3  },
    { bin_op_type::l_xor, 3  },
    { bin_op_type::b_or,  4  },
    { bin_op_type::b_xor, 5  },
    { bin_op_type::b_and, 6  },
    { bin_op_type::eq,    7  },
    { bin_op_type::neq,   7  },
    { bin_op_type::lt,    8  },
    { bin_op_type::gt,    8  },
    { bin_op_type::lte,   8  },
    { bin_op_type::gte,   8  },
    { bin_op_type::add,   10 },
    { bin_op_type::sub,   10 },
    { bin_op_type::mul,   20 },
    { bin_op_type::div,   20 },
    { bin_op_type::mod,   20 },
    { bin_op_type::shl,   20 },
    { bin_op_type::shr,   20 },
    { bin_op_type::pow,   30 }
};

const std::unordered_map<std::string_view, bin_op_type> pm::binop_type_map {
    { "||", bin_op_type::l_or  },
    { "&&", bin_op_type::l_and },
    { "|" , bin_op_type::b_or  },
    { "^" , bin_op_type::b_xor },
    { "&" , bin_op_type::b_and },
    { "==", bin_op_type::eq    },
    { "!=", bin_op_type::neq   },
    { "<" , bin_op_type::lt    },
    { ">" , bin_op_type::gt    },
    { "<=", bin_op_type::lte   },
    { ">=", bin_op_type::gte   },
    { "+" , bin_op_type::add   },
    { "-" , bin_op_type::sub   },
    { "*" , bin_op_type::mul   },
    { "/" , bin_op_type::div   },
    { "%" , bin_op_type::mod   },
    { "<<", bin_op_type::shl   },
    { ">>", bin_op_type::shr   },
    { "**", bin_op_type::pow   },
    { ".",  bin_op_type::acc   },
    { "->", bin_op_type::accdf },
};

const std::unordered_map<std::string_view, un_op_type> pm::unop_type_map {
    {"*", un_op_type::deref     },
    {"&", un_op_type::addr_of   },
    {"!", un_op_type::log_not   },
    {"-", un_op_type::negate    },
    {"~", un_op_type::bit_not   }
};

const std::unordered_map<std::string_view, intrinsic_type> pm::intrin_map {
    { "i8",   intrinsic_type::i8    },
    { "i16",  intrinsic_type::i16   },
    { "i32",  intrinsic_type::i32   },
    { "i64",  intrinsic_type::i64   },

    { "u8",   intrinsic_type::u8    },
    { "u16",  intrinsic_type::u16   },
    { "u32",  intrinsic_type::u32   },
    { "u64",  intrinsic_type::u64   },

    { "f32",  intrinsic_type::f32   },
    { "f64",  intrinsic_type::f64   },

    { "char", intrinsic_type::char_ },
    { "bool", intrinsic_type::bool_ },
    { "void", intrinsic_type::void_ }
};

const std::unordered_map<std::string_view, assn_type> pm::assign_type_map {
    { "=",  assn_type::none },
    { "+=", assn_type::plus_eq    },
    { "-=", assn_type::minus_eq   },
    { "*=", assn_type::mul_eq     },
    { "/=", assn_type::div_eq     },
    { "%=", assn_type::mod_eq     },
    { "<<=", assn_type::shl_eq    },
    { ">>=", assn_type::shr_eq    },
    { "&=",  assn_type::b_and_eq    },
    { "^=",  assn_type::b_xor_eq    },
    { "|=",  assn_type::b_or_eq     }
};

const std::unordered_map<ast::nodes::bin_op_type, llvm::Instruction::BinaryOps> pm::basic_binop_map {
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

const std::unordered_map<ast::nodes::bin_op_type, llvm::CmpInst::Predicate> pm::i_cmp_map {
    { ast::nodes::bin_op_type::eq, llvm::CmpInst::Predicate::ICMP_EQ },
    { ast::nodes::bin_op_type::neq, llvm::CmpInst::Predicate::ICMP_NE },
    { ast::nodes::bin_op_type::lt, llvm::CmpInst::Predicate::ICMP_ULT },
    { ast::nodes::bin_op_type::gt, llvm::CmpInst::Predicate::ICMP_UGT },
    { ast::nodes::bin_op_type::lte, llvm::CmpInst::Predicate::ICMP_ULE },
    { ast::nodes::bin_op_type::gte, llvm::CmpInst::Predicate::ICMP_UGE },
};

const std::unordered_map<ast::nodes::bin_op_type, llvm::CmpInst::Predicate> pm::f_cmp_map {
    { ast::nodes::bin_op_type::eq, llvm::CmpInst::Predicate::FCMP_OEQ },
    { ast::nodes::bin_op_type::neq, llvm::CmpInst::Predicate::FCMP_ONE },
    { ast::nodes::bin_op_type::lt, llvm::CmpInst::Predicate::FCMP_OLT },
    { ast::nodes::bin_op_type::gt, llvm::CmpInst::Predicate::FCMP_OGT },
    { ast::nodes::bin_op_type::lte, llvm::CmpInst::Predicate::FCMP_OLE },
    { ast::nodes::bin_op_type::gte, llvm::CmpInst::Predicate::FCMP_OGE },
};