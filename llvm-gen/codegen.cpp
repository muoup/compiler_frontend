#include "codegen.h"

#include <charconv>
#include <stdexcept>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include "libc_ref.h"
#include "operators.h"
#include "types.h"
#include "../ast/declarations.h"

using namespace cg_llvm;

static llvm::LLVMContext context;

using var_table = std::unordered_map<std::string_view, llvm::Type*>;

llvm::AllocaInst *scope_data::add_variable(const std::string_view name, llvm::Type *type) const {
    if (tables.back()->contains(name))
        throw std::runtime_error("Variable already exists in symbol table.");

    auto* alloca = builder.CreateAlloca(type);
    alloca->setName(name);

    tables.back()->emplace(name, alloca);
    return get_variable(name);
}

llvm::AllocaInst *scope_data::get_variable(const std::string_view name) const {
    for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
        if ((*it)->contains(name))
            return (*it)->at(name);
    }

    throw std::runtime_error("Variable not found in symbol table.");
}

llvm::Value* cg_llvm::generate_literal(const ast::ast_node& node, scope_data& scope) {
    auto& data = node.data;

    switch (node.type) {
        case ast::ast_node_type::INT_LITERAL:
            int i_val;
            std::from_chars(data.data(), data.data() + data.size(), i_val);
            return llvm::ConstantInt::get(context,
                llvm::APInt(32, i_val)
                );
        case ast::ast_node_type::FLOAT_LITERAL:
            float f_val;
            std::from_chars(data.data(), data.data() + data.size(), f_val);
            return llvm::ConstantFP::get(context,
                llvm::APFloat(f_val)
                );
        case ast::ast_node_type::CHAR_LITERAL:
            return llvm::ConstantInt::get(context,
                llvm::APInt(8, data[0])
                );
        case ast::ast_node_type::STRING_LITERAL:
            return scope.builder.CreateGlobalStringPtr(data);
        default:
            std::unreachable();
    }
}

llvm::Value *cg_llvm::generate_initialization(const ast::ast_node &node, scope_data &scope) {
    if (node.type != ast::ast_node_type::INITIALIZATION)
        throw std::runtime_error("Invalid node type for initialization generation.");

    return scope.add_variable(node.data, get_llvm_type(node, context));
}

llvm::Value* cg_llvm::generate_method_call(const ast::ast_node& node, scope_data& scope) {
    const auto& arg_list = node.children[0];

    std::vector<llvm::Value*> args;
    for (auto& child : arg_list.children) {
        args.emplace_back(generate_statement(child, scope));
    }

    llvm::Function* func = node.data.starts_with(libc_prefix) ?
        get_libc_fn(node.data, scope) : scope.root->getFunction(node.data);

    if (!func)
        throw std::runtime_error("Function not found.");

    if (arg_list.children.size() != func->arg_size() && !func->isVarArg())
        throw std::runtime_error("Argument count mismatch.");

    return scope.builder.CreateCall(func, args);
}

llvm::Value* cg_llvm::generate_expression(const ast::ast_node& node, scope_data& scope) {
    llvm::AllocaInst* var;

    switch (node.type) {
        case ast::ast_node_type::VARIABLE:
            var = scope.get_variable(node.data);
            return scope.builder.CreateLoad(var->getType(), var, node.data);

        case ast::ast_node_type::METHOD_CALL:
            return generate_method_call(node, scope);

        case ast::ast_node_type::INITIALIZATION:
            return generate_initialization(node, scope);

        case ast::ast_node_type::BIN_OP:
            return generate_binop(node, scope);

        case ast::ast_node_type::INT_LITERAL:
        case ast::ast_node_type::FLOAT_LITERAL:
        case ast::ast_node_type::CHAR_LITERAL:
        case ast::ast_node_type::STRING_LITERAL:
            // Generate the value
            return generate_literal(node, scope);

        default:
            throw std::runtime_error("Invalid node type for expression generation.");
    }
}

llvm::Value* cg_llvm::generate_return(const ast::ast_node& node, scope_data& data) {
    if (node.type != ast::ast_node_type::RETURN)
        throw std::runtime_error("Invalid node type for return generation.");

    llvm::Value* ret_val = nullptr;

    if (!node.children.empty()) {
        const auto& ret_node = node.children.front();

        if (ret_node.type != ast::ast_node_type::EXPRESSION)
            throw std::runtime_error("Invalid node type for return value.");

        ret_val = generate_expression(ret_node.children.front(), data);
        ret_val = attempt_cast(ret_val, data.current_function->getReturnType(), data);
    }

    return data.builder.CreateRet(ret_val);
}

llvm::Value* cg_llvm::generate_statement(const ast::ast_node& node, scope_data& data) {
    if (node.type == ast::ast_node_type::EXPRESSION)
        return generate_expression(node.children.front(), data);
    else if (node.type == ast::ast_node_type::RETURN)
        return generate_return(node, data);

    throw std::runtime_error("Invalid node type for statement generation.");
}

void cg_llvm::generate_block(const ast::ast_node& node, const scope_data& data) {
    if (node.type != ast::ast_node_type::CODE_BLOCK)
        throw std::runtime_error("Method body is not a block.");

    scope_data new_scope = data;
    var_table table;
    new_scope.tables.push_back(&table);

    for (auto& children : node.children)
        generate_statement(children, new_scope);
}

llvm::Function* cg_llvm::generate_method(const ast::ast_node& node, std::shared_ptr<llvm::Module> root) {
    llvm::Type *type = get_llvm_type(node, context);

    const auto &param_list = node.children[0];
    std::vector<llvm::Type*> param_types;

    // Add parameters to function
    param_types.reserve(param_list.children.size());
    for (const auto &param : param_list.children) {
        param_types.emplace_back(get_llvm_type(param, context));
    }

    llvm::FunctionType *func_type = llvm::FunctionType::get(type, { param_types }, false);
    llvm::Function *func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, node.data, root.get());

    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", func);
    llvm::IRBuilder<> builder(entry);

    for (auto i = 0; i < param_list.children.size(); ++i) {
        const auto& param_name = param_list.children[i].data;
        const auto arg = func->arg_begin() + i;

        arg->setName(param_name);
    }

    const scope_data scope_data {
        .context = context,
        .root = root,
        .builder = builder,
        .tables = { },
        .current_function = func,
    };

    generate_block(node.children[1], scope_data);

    return func;
}

void cg_llvm::generate_code(llvm::raw_ostream &out, const ast::ast_node &root) {
    if (root.type != ast::ast_node_type::ROOT)
        throw std::runtime_error("Passed non-root node to codegen.");

    std::shared_ptr llvm_module = std::make_shared<llvm::Module>("top", context);

    for (const auto &child : root.children) {
        generate_method(child, llvm_module);
    }

    out << *llvm_module;
}
