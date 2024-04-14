#pragma once

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include "../ast/data/ast_nodes.h"

namespace cg {
    struct struct_definition {
        llvm::StructType* struct_type;
        std::vector<std::string_view> field_names;
    };

    struct scope_variable {
        llvm::AllocaInst* var_allocation;
        const struct_definition* struct_type = nullptr;
        bool is_const;
    };

    using var_table = std::unordered_map<std::string_view, scope_variable>;

    struct scope_data {
        llvm::LLVMContext &context;
        std::shared_ptr<llvm::Module> module;
        llvm::IRBuilder<> &builder;
        std::vector<std::shared_ptr<var_table>> var_tables;
        std::shared_ptr<std::unordered_map<std::string_view, struct_definition>> struct_table;

        llvm::BasicBlock *entry = nullptr;
        llvm::Function* current_function = nullptr;

        const scope_variable& get_variable(std::string_view name) const;
        const struct_definition& get_struct(std::string_view name) const;
    };

    void generate_code(const ast::nodes::root &root, llvm::raw_ostream &ostream);
}