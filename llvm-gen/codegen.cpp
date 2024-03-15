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
#include "../ast/data/ast_nodes.h"
#include "../ast/declarations.h"

using namespace cg_llvm;

static llvm::LLVMContext context;

using var_table = std::unordered_map<std::string_view, llvm::Type*>;

const scope_variable &scope_data::add_variable(const std::string_view name, llvm::Type *type, const bool const_) const {
    if (tables.back()->contains(name))
        throw std::runtime_error("Variable already exists in symbol table.");

    auto* var_alloca = builder.CreateAlloca(type, nullptr, name);

    tables.back()->emplace(name, scope_variable { var_alloca, const_ });
    return get_variable(name);
}

const scope_variable &scope_data::get_variable(const std::string_view name) const {
    for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
        if ((*it)->contains(name))
            return (*it)->at(name);
    }

    throw std::runtime_error("Variable not found in symbol table.");
}

llvm::Value* cg_llvm::generate_literal(const ast::nodes::literal& node, const scope_data& scope) {
    auto& val = node.value;

    switch (val.index()) {
        using namespace ast::nodes;
        case UINT:
            return llvm::ConstantInt::get(context,
                llvm::APInt(32, std::get<unsigned int>(val))
                );
        case INT:
            return llvm::ConstantInt::get(context,
                llvm::APInt(32, std::get<int>(val))
                );
        case DOUBLE:
            return llvm::ConstantFP::get(context,
                llvm::APFloat(std::get<double>(val))
                );
        case CHAR:
            return llvm::ConstantInt::get(context,
                llvm::APInt(8, std::get<char>(val))
                );
        case STRING:
            return scope.builder.CreateGlobalStringPtr(
                std::get<std::string_view>(val)
                );
        default:
            std::unreachable();
    }
}

llvm::Value *cg_llvm::generate_initialization(const ast::nodes::initialization &init, scope_data &scope) {
    const auto &alloc = scope.add_variable(init.var.name, get_llvm_type(init.type, context));

    return std::get<llvm::AllocaInst*>(alloc.var_allocation);
}

llvm::Value *cg_llvm::generate_const_init(const ast::nodes::const_assignment &init, scope_data &scope) {
    const auto& value = generate_expression(*init.value, scope);

    // if (auto *const_data = llvm::dyn_cast<llvm::Constant>(value)) {
    //     scope.tables.front()->emplace(
    //         init.variable.var.name,
    //         scope_variable { const_data, true }
    //     );
    // }

    auto *val = generate_initialization(init.variable, scope);
    return scope.builder.CreateStore(value, val);
}

llvm::Value* cg_llvm::generate_method_call(const ast::nodes::method_call &method_call, scope_data &scope) {
    std::vector<llvm::Value*> args;
    for (auto& child : method_call.arguments) {
        args.emplace_back(generate_expression(child, scope));
    }

    const std::string_view method_name = method_call.method_name;

    llvm::Function* func = method_name.starts_with(libc_prefix) ?
        get_libc_fn(method_name, scope) : scope.root->getFunction(method_name);

    if (!func)
        throw std::runtime_error("Function not found.");

    if (method_call.arguments.size() != func->arg_size() && !func->isVarArg())
        throw std::runtime_error("Argument count mismatch.");

    return scope.builder.CreateCall(func, args);
}

llvm::Value* cg_llvm::generate_variable(const ast::nodes::variable& var, scope_data& scope) {
    const auto scope_var = scope.get_variable(var.name);

    if (scope_var.var_allocation.index() == 0) {
        return std::get<llvm::Constant*>(scope_var.var_allocation);
    }

    if (scope_var.var_allocation.index() == 1) {
        auto *alloca_ = std::get<llvm::AllocaInst*>(scope_var.var_allocation);

        return scope.builder.CreateLoad(alloca_->getAllocatedType(), alloca_);
    }

    throw std::runtime_error("Invalid variable allocation type.");
}

llvm::Value* cg_llvm::generate_expression(const ast::nodes::expression& node, scope_data& scope) {
    switch (node.value.index()) {
        using namespace ast::nodes;
        case VARIABLE:
            return generate_variable(std::get<variable>(node.value), scope);

        case METHOD_CALL:
            return generate_method_call(std::get<method_call>(node.value), scope);

        case ASSIGNMENT:
            return generate_assignment(std::get<assignment>(node.value), scope);

        case INITIALIZATION:
            return generate_initialization(std::get<initialization>(node.value), scope);

        case CONST_ASSIGNMENT:
            return generate_const_init(std::get<const_assignment>(node.value), scope);

        case UN_OP:
            return generate_unop(std::get<un_op>(node.value), scope);

        case BIN_OP:
            return generate_binop(std::get<bin_op>(node.value), scope);

        case LITERAL:
            return generate_literal(std::get<literal>(node.value), scope);

        default:
            throw std::runtime_error("Invalid node type for expression generation.");
    }
}

llvm::Value* cg_llvm::generate_return(const ast::nodes::return_op& ret_op, scope_data& data) {
    llvm::Value* ret_val = nullptr;

    if (ret_op.value) {
        ret_val = generate_expression(ret_op.value.value(), data);
        ret_val = attempt_cast(ret_val, data.current_function->getReturnType(), data);
    }

    return data.builder.CreateRet(ret_val);
}

llvm::Value* cg_llvm::generate_statement(const ast::nodes::statement &stmt, scope_data& data) {
    switch (stmt.value.index()) {
        using namespace ast::nodes;
        case CONDITIONAL:
            throw std::runtime_error("Conditionals not implemented.");
        case EXPRESSION:
            return generate_expression(std::get<expression>(stmt.value), data);
        case RETURN:
            return generate_return(std::get<return_op>(stmt.value), data);
        default:
            std::unreachable();
    }
}

void cg_llvm::generate_block(const ast::nodes::code_block &block, const scope_data& data) {
    scope_data new_scope = data;
    var_table table;
    new_scope.tables.push_back(&table);

    for (auto& children : block.statements)
        generate_statement(children, new_scope);
}

llvm::Function* cg_llvm::generate_method(const ast::nodes::function &fn, std::shared_ptr<llvm::Module> root) {
    llvm::Type *type = get_llvm_type(fn.return_type, context);

    std::vector<llvm::Type*> param_types;

    // Add parameters to function
    param_types.reserve(fn.param_types.size());
    for (const auto &param : fn.param_types) {
        param_types.emplace_back(get_llvm_type(param.type, context));
    }

    llvm::FunctionType *func_type = llvm::FunctionType::get(type, { param_types }, false);
    llvm::Function *func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, fn.function_name, root.get());

    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", func);
    llvm::IRBuilder<> builder(entry);

    for (auto i = 0; i < fn.param_types.size(); ++i) {
        const auto& method_param = fn.param_types[i];
        const auto arg = func->arg_begin() + i;

        arg->setName(method_param.var.name);
    }

    const scope_data scope_data {
        .context = context,
        .root = root,
        .builder = builder,
        .tables = { },
        .current_function = func,
    };

    generate_block(fn.body, scope_data);

    return func;
}

void cg_llvm::generate_code(llvm::raw_ostream &out, const ast::nodes::root &root) {
    std::shared_ptr llvm_module = std::make_shared<llvm::Module>("top", context);

    for (const auto &child : root.functions) {
        generate_method(child, llvm_module);
    }

    out << *llvm_module;
}
