#pragma once
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>
#include <llvm/IR/IRBuilder.h>

namespace ast::nodes {
    struct var_ref;
    struct initialization;
}

namespace llvm {
    class Type;
    class LLVMContext;
    class Module;
    class Function;
    class Value;
    class raw_ostream;
}

namespace ast::nodes {
    struct root;
    struct literal;
    struct method_call;
    struct expression;
    struct bin_op;
    struct un_op;
    struct statement;
    struct return_op;
    struct function;
    struct type_instance;
    struct code_block;
}

namespace cg_llvm {
    struct scope_variable {
        llvm::AllocaInst* var_allocation;
        bool is_const;
    };

    using var_table = std::unordered_map<std::string_view, scope_variable>;

    struct scope_data {
        llvm::LLVMContext &context;
        std::shared_ptr<llvm::Module> root;
        llvm::IRBuilder<> &builder;
        std::vector<var_table*> tables;

        llvm::Function* current_function = nullptr;

        const scope_variable &add_variable(std::string_view name, llvm::Type *type, bool const_ = false) const;
        const scope_variable &get_variable(std::string_view name) const;
    };

    void generate_code(llvm::raw_ostream &out, const ast::nodes::root & root);

    llvm::Value* generate_literal(const ast::nodes::literal &node, const scope_data& scope);
    llvm::Value* generate_method_call(const ast::nodes::method_call &method_call, scope_data& scope);
    llvm::Value* generate_const_init(const ast::nodes::initialization &init, scope_data& scope);
    llvm::Value* generate_variable(const ast::nodes::var_ref & var, const scope_data& scope);
    llvm::Value* generate_expression(const ast::nodes::expression &node, scope_data& scope);
    llvm::Value* generate_binop(const ast::nodes::bin_op &node, scope_data &data);
    llvm::Value* generate_statement(const ast::nodes::statement &stmt, scope_data& data);
    llvm::Value* generate_return(const ast::nodes::return_op &ret_op, scope_data& data);

    llvm::Value* generate_store(llvm::AllocaInst *var, llvm::Value *value, scope_data& scope);
    llvm::Value* generate_load(llvm::AllocaInst *var, const scope_data& scope);
    llvm::Value* varargs_cast(llvm::Value *val, const scope_data& scope);

    llvm::Function* generate_method(const ast::nodes::function &node, std::shared_ptr<llvm::Module> root);

    llvm::AllocaInst *generate_initialization(const ast::nodes::type_instance &init, scope_data &scope);
    void generate_block(const ast::nodes::code_block &node, const scope_data& data);
}
