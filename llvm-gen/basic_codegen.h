#pragma once

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include "../ast/data/ast_nodes.h"

namespace cg {
    struct scope_variable {
        llvm::AllocaInst* var_allocation;
        bool is_const;
    };

    using var_table = std::unordered_map<std::string_view, scope_variable>;

    struct scope_data {
        llvm::LLVMContext &context;
        std::shared_ptr<llvm::Module> module;
        llvm::IRBuilder<> &builder;
        std::vector<std::shared_ptr<var_table>> tables;

        llvm::BasicBlock *entry = nullptr;
        llvm::Function* current_function = nullptr;

        const scope_variable& add_variable(std::string_view name, llvm::Type *type, bool const_ = false) const;
        const scope_variable& get_variable(std::string_view name) const;
    };

    void generate_code(const ast::nodes::root &root, llvm::raw_ostream &ostream);
}