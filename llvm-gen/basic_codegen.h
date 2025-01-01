#pragma once

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include "../ast/data/ast_nodes.h"

namespace cg {
    struct struct_definition {
        llvm::StructType* struct_type;
        std::vector<ast::nodes::type_instance> field_decls;
    };

    struct scope_variable {
        llvm::Value* var_allocation;
        const struct_definition* struct_type = nullptr;
        bool is_const;
    };

    using var_table = std::unordered_map<std::string_view, scope_variable>;

    struct scope_data {
        llvm::LLVMContext &context;
        llvm::Module* module;
        llvm::IRBuilder<> &builder;
        std::vector<std::shared_ptr<var_table>> var_tables;
        std::shared_ptr<std::unordered_map<std::string_view, struct_definition>> struct_table;

        llvm::BasicBlock *block = nullptr;
        llvm::Function* current_function = nullptr;

        const scope_variable& get_variable(std::string_view name) const;
        const struct_definition& get_struct(std::string_view name) const;
    };

    struct ir_data {
        std::unique_ptr<llvm::Module> module;
        std::unique_ptr<llvm::LLVMContext> context;
    };

    ir_data generate_ir(const ast::nodes::root &root);
}