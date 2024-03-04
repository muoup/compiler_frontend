#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <llvm/IR/IRBuilder.h>

namespace llvm {
    class Type;
    class LLVMContext;
    class Module;
    class Function;
    class Value;
    class raw_ostream;
}

namespace ast {
    struct ast_node;
}

namespace cg_llvm {
    using var_table = std::unordered_map<std::string_view, llvm::Value*>;

    struct scope_data {
        llvm::LLVMContext &context;
        std::shared_ptr<llvm::Module> root;
        llvm::IRBuilder<> &builder;
        std::vector<var_table*> tables;

        llvm::Value* add_variable(std::string_view name, llvm::Type* type);
        llvm::Value* get_variable(std::string_view name) const;
    };

    void generate_code(llvm::raw_ostream &out, const ast::ast_node &root);

    llvm::Value* generate_literal(const ast::ast_node& node, scope_data& data);
    llvm::Value* generate_method_call(const ast::ast_node& node, scope_data& scope);
    llvm::Value* generate_expression(const ast::ast_node& node, scope_data& scope);
    llvm::Value* generate_binop(const ast::ast_node &node, scope_data &data);
    llvm::Value* generate_statement(const ast::ast_node& node, scope_data& data);
    llvm::Value* generate_return(const ast::ast_node& node, scope_data& data);

    llvm::Function* generate_method(const ast::ast_node& node, std::shared_ptr<llvm::Module> root);

    llvm::Value *generate_initialization(const ast::ast_node &node, scope_data &scope);
    void generate_block(const ast::ast_node& node, const scope_data& data);
}
