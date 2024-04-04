#include "basic_codegen.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>

#include "../ast/data/data_maps.h"
#include "types.h"
#include "libc_ref.h"
#include "operators.h"

using namespace ast::nodes;
using namespace cg;

const scope_variable& scope_data::add_variable(std::string_view name, llvm::Type *type, bool const_) const {
    if (tables.back()->contains(name))
        throw std::runtime_error("Variable already exists in symbol table.");

    auto* alloca_inst = builder.CreateAlloca(type);
    alloca_inst->setName(name);

    tables.back()->emplace(name, scope_variable { alloca_inst, const_ });
    return get_variable(name);
}

const scope_variable& scope_data::get_variable(std::string_view name) const {
    for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
        if ((*it)->contains(name))
            return (*it)->at(name);
    }

    throw std::runtime_error("Variable not found in symbol table.");
}

void cg::generate_code(llvm::raw_ostream &out, const ast::nodes::root &root) {
    llvm::LLVMContext context;
    auto module = std::make_shared<llvm::Module>("main", context);
    llvm::IRBuilder<> builder(context);

    scope_data scope {
        context,
        module,
        builder,
        std::vector { std::make_shared<var_table>() }
    };

    root.generate_code(scope);
    module->print(out, nullptr);
}

llvm::Value* root::generate_code(cg::scope_data &scope) const {
//    for (const auto &var : global_vars)
//        var.generate_code(scope);

    for (const auto &fn : functions)
        fn.generate_code(scope);

    return nullptr;
}

llvm::Value* literal::generate_code(cg::scope_data &scope) const {
    switch (this->value.index()) {
        using namespace ast::nodes;
        case UINT:
            return llvm::ConstantInt::get(scope.context,
                    llvm::APInt(type_size, std::get<unsigned int>(value))
            );
        case INT:
            return llvm::ConstantInt::get(scope.context,
                    llvm::APInt(type_size, std::get<int>(value))
            );
        case FLOAT:
            return llvm::ConstantFP::get(scope.context,
                    llvm::APFloat(std::get<double>(value))
            );
        case CHAR:
            return llvm::ConstantInt::get(scope.context,
                    llvm::APInt(8, std::get<char>(value))
            );
        case STRING:
            return scope.builder.CreateGlobalStringPtr(
                    std::get<std::string_view>(value)
            );
        default:
            std::unreachable();
    }
}

llvm::Value* initialization::generate_code(cg::scope_data &scope) const {
    return scope.add_variable(
            variable.var_name,
            get_llvm_type(variable.type, scope.context)
    ).var_allocation;
}

llvm::Value* method_call::generate_code(cg::scope_data &scope) const {
    llvm::Function* func = method_name.starts_with(libc_prefix) ?
                           get_libc_fn(method_name, scope) : scope.root->getFunction(method_name);

    std::vector<llvm::Value*> args;

    for (auto i = 0; i < arguments.size(); ++i) {
        auto child = arguments[i]->generate_code(scope);

        if (i < func->arg_size())
            child = attempt_cast(child, func->getArg(i)->getType(), scope);
        else if (i >= func->arg_size() && !func->isVarArg())
            throw std::runtime_error("Argument count mismatch.");
        else
            child = varargs_cast(child, scope);

        args.emplace_back(child);
    }

    if (!func)
        throw std::runtime_error("Function not found.");

    if (arguments.size() != func->arg_size() && !func->isVarArg())
        throw std::runtime_error("Argument count mismatch.");

    return scope.builder.CreateCall(func, args);
}

llvm::Value* var_ref::generate_code(cg::scope_data &scope) const {
    return scope.get_variable(name).var_allocation;
}

llvm::Value* return_op::generate_code(cg::scope_data &scope) const {
    return scope.builder.CreateRet(val ? val->generate_code(scope) : nullptr);
}

llvm::Value* scope_block::generate_code(cg::scope_data &scope) const {
    for (const auto &stmt : statements)
        stmt->generate_code(scope);

    return nullptr;
}

llvm::Value* function::generate_code(cg::scope_data &scope) const {
    llvm::Type *type = get_llvm_type(this->return_type, scope.context);

    std::vector<llvm::Type*> params;

    params.reserve(param_types.size());
    for (const auto &param : param_types)
        params.emplace_back(get_llvm_type(param.type, scope.context));

    llvm::FunctionType *func_type = llvm::FunctionType::get(type, llvm::ArrayRef<llvm::Type*> { params }, false);
    llvm::Function *func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, fn_name, *scope.root);

    llvm::BasicBlock *entry = llvm::BasicBlock::Create(scope.context, "entry", func);
    llvm::IRBuilder<> builder(entry);

    scope_data body_scope {
        scope.context,
        scope.root,
        builder,
        scope.tables
    };
    body_scope.current_function = func;

    for (auto i = 0; i < param_types.size(); ++i) {
        auto arg = func->arg_begin() + i;
        arg->setName(param_types[i].var_name);

        body_scope.add_variable(param_types[i].var_name, arg->getType());
    }

    body.generate_code(body_scope);
    return func;
}

llvm::Value* bin_op::generate_code(cg::scope_data &scope) const {
    const auto [lhs, rhs, is_int] = balance_sides(
            left->generate_code(scope),
            right->generate_code(scope),
            scope);
    const auto bin_op_type = static_cast<llvm::Instruction::BinaryOps>(ast::pm::binop_map.at(type) + is_int);
    return scope.builder.CreateBinOp(bin_op_type, lhs, rhs);
}

llvm::Value* un_op::generate_code(cg::scope_data &scope) const {
    auto *val = value->generate_code(scope);

    switch (type) {
        using namespace ast::nodes;
        case un_op_type::log_not:
            return scope.builder.CreateNot(val);
        case un_op_type::deref:
            if (!val->getType()->isPointerTy())
                throw std::runtime_error("Cannot dereference non-pointer type.");

            return scope.builder.CreateLoad(val->getType(), val);
        case un_op_type::addr_of:
            return value->generate_code(scope);
        case un_op_type::bit_not:
            if (!val->getType()->isIntegerTy())
                throw std::runtime_error("Cannot perform bitwise NOT on non-integer type.");

            return scope.builder.CreateBinOp(
                    llvm::Instruction::BinaryOps::Xor,
                    val, llvm::ConstantInt::get(val->getType(), -1)
            );
        case un_op_type::negate:
            if (val->getType()->isIntegerTy())
                return scope.builder.CreateNeg(val);
            else if (val->getType()->isFloatingPointTy())
                return scope.builder.CreateFNeg(val);
            else
                throw std::runtime_error("Cannot negate non-numeric type.");
        default:
            throw std::runtime_error("Unknown unary operator.");
    }
}

llvm::Value* assignment::generate_code(cg::scope_data &scope) const {
    return scope.builder.CreateStore(
        rhs->generate_code(scope),
        lhs->generate_code(scope)
    );
}

llvm::Value* conditional::generate_code(cg::scope_data &scope) const {
    return nullptr;
}