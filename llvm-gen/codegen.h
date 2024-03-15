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

namespace ast::nodes {
    struct variable;
    struct root;
    struct literal;
    struct method_call;
    struct expression;
    struct bin_op;
    struct un_op;
    struct statement;
    struct return_op;
    struct function;
    struct initialization;
    struct code_block;
}

namespace cg_llvm {
    using var_table = std::unordered_map<std::string_view, llvm::AllocaInst*>;

    struct scope_data {
        llvm::LLVMContext &context;
        std::shared_ptr<llvm::Module> root;
        llvm::IRBuilder<> &builder;
        std::vector<var_table*> tables;

        llvm::Function* current_function = nullptr;

        llvm::AllocaInst *add_variable(std::string_view name, llvm::Type *type) const;
        llvm::AllocaInst *get_variable(std::string_view name) const;
    };

    void generate_code(llvm::raw_ostream &out, const ast::nodes::root & root);

    llvm::Value* generate_literal(const ast::nodes::literal &node, const scope_data& scope);
    llvm::Value* generate_method_call(const ast::nodes::method_call &method_call, scope_data& scope);
    llvm::Value* generate_variable(const ast::nodes::variable &var, const scope_data& scope);
    llvm::Value* generate_expression(const ast::nodes::expression &node, scope_data& scope);
    llvm::Value* generate_binop(const ast::nodes::bin_op &node, scope_data &data);
    llvm::Value* generate_statement(const ast::nodes::statement &stmt, scope_data& data);
    llvm::Value* generate_return(const ast::nodes::return_op &ret_op, scope_data& data);

    llvm::Function* generate_method(const ast::nodes::function &node, std::shared_ptr<llvm::Module> root);

    llvm::Value *generate_initialization(const ast::nodes::initialization &init, scope_data &scope);
    void generate_block(const ast::nodes::code_block &node, const scope_data& data);
}
