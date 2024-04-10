#include "basic_codegen.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>

#include "../ast/data/data_maps.h"
#include "types.h"
#include "libc_ref.h"
#include "operators.h"

using namespace ast::nodes;
using namespace cg;

scope_data gen_inner_scope(scope_data &scope) {
    scope_data data = scope;
    data.entry = llvm::BasicBlock::Create(data.context, "entry", data.current_function);
    data.builder.SetInsertPoint(data.entry);
    data.tables.emplace_back(std::make_shared<var_table>());
    return data;
}

const scope_variable& scope_data::add_variable(std::string_view name, llvm::Type *type, bool const_) const {
    if (tables.back()->contains(name))
        throw std::runtime_error("Variable already exists in symbol table.");

    auto* alloca_inst = builder.CreateAlloca(type);
    alloca_inst->setName(name);

    tables.back()->emplace(
        name,
        scope_variable { alloca_inst, const_ }
    );

    return get_variable(name);
}

const scope_variable& scope_data::get_variable(std::string_view name) const {
    for (auto it = tables.rbegin(); it != tables.rend(); ++it) {
        if ((*it)->contains(name))
            return (*it)->at(name);
    }

    throw std::runtime_error("Variable not found in symbol table.");
}

void cg::generate_code(const ast::nodes::root &root, llvm::raw_ostream &ostream) {
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
    module->print(ostream, nullptr);
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
                           get_libc_fn(method_name, scope) :
                           scope.module->getFunction(method_name);

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
    auto var = scope.get_variable(name);
    auto alloc_type = var.var_allocation->getAllocatedType();

    return scope.builder.CreateLoad(
        alloc_type,
        var.var_allocation
    );
}

llvm::Value *raw_var::generate_code(cg::scope_data &scope) const {
    return scope.get_variable(name).var_allocation;
}

llvm::Value* return_op::generate_code(cg::scope_data &scope) const {
    if (val) {
        auto ret_val = val->generate_code(scope);
        auto ret_type = scope.current_function->getReturnType();

        if (ret_val->getType() != ret_type)
            ret_val = attempt_cast(ret_val, ret_type, scope);

        return scope.builder.CreateRet(ret_val);
    }

    return scope.builder.CreateRetVoid();
}

llvm::Value* conditional_expression(const std::unique_ptr<ast::nodes::expression> &condition, cg::scope_data &scope) {
    auto cond = condition->generate_code(scope);
    return attempt_cast(cond, llvm::Type::getInt1Ty(scope.context), scope);
}

llvm::Value *if_statement::generate_code(cg::scope_data &scope) const {
    auto cond = conditional_expression(condition, scope);

    llvm::Function *func = scope.builder.GetInsertBlock()->getParent();
    auto pre_insert = scope.builder.GetInsertBlock();

    auto *then_block = body.generate_code(scope);
    auto *else_block = else_body ? else_body->generate_code(scope) : nullptr;
    auto *merge_block = llvm::BasicBlock::Create(scope.context, "merge", func);

    scope.builder.SetInsertPoint(then_block);
    scope.builder.CreateBr(merge_block);

    if (else_block) {
        scope.builder.SetInsertPoint(else_block);
        scope.builder.CreateBr(merge_block);
    }

    scope.builder.SetInsertPoint(pre_insert);
    scope.builder.CreateCondBr(cond, then_block, else_block ? else_block : merge_block);

    scope.builder.SetInsertPoint(merge_block);
    return nullptr;
}

llvm::Value* loop::generate_code(cg::scope_data &scope) const {
    const auto pre_init = scope.builder.GetInsertBlock();

    auto *block = body.generate_code(scope);
    auto *cond_block = llvm::BasicBlock::Create(scope.context, "cond", scope.current_function);
    auto *ret_block = llvm::BasicBlock::Create(scope.context, "ret", scope.current_function);
    scope.builder.CreateBr(cond_block);

    scope.builder.SetInsertPoint(cond_block);
    auto cond = conditional_expression(condition, scope);
    scope.builder.CreateCondBr(cond, block, ret_block);

    if (pre_eval) {
        scope.builder.SetInsertPoint(pre_init);
        scope.builder.CreateBr(cond_block);
        scope.builder.SetInsertPoint(cond_block);
    }

    scope.builder.SetInsertPoint(ret_block);

    return nullptr;
}

llvm::BasicBlock* scope_block::generate_code(cg::scope_data &scope) const {
    auto in_scope_block = gen_inner_scope(scope);

    for (const auto &stmt : statements)
        stmt->generate_code(in_scope_block);

    return in_scope_block.builder.GetInsertBlock();
}

llvm::Value* function::generate_code(cg::scope_data &scope) const {
    llvm::Type *type = get_llvm_type(this->return_type, scope.context);

    std::vector<llvm::Type*> params;

    params.reserve(param_types.size());
    for (const auto &param : param_types)
        params.emplace_back(get_llvm_type(param.type, scope.context));

    llvm::FunctionType *func_type = llvm::FunctionType::get(type, llvm::ArrayRef<llvm::Type*> { params }, false);
    llvm::Function *func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, fn_name, *scope.module);

    scope_data param_scope = gen_inner_scope(scope);
    param_scope.current_function = func;

    for (auto i = 0; i < param_types.size(); ++i) {
        auto arg = func->args().begin() + i;
        arg->setName(param_types[i].var_name);

//        body_scope.tables.back()->emplace(
//            param_types[i].var_name,
//            scope_variable { arg, true }
//        );
    }

    body.generate_code(param_scope);
    return func;
}

llvm::Value* bin_op::generate_code(cg::scope_data &scope) const {
    return cg::generate_binop(*this, scope);
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
    auto balance = balance_sides(
        lhs->generate_code(scope),
        rhs->generate_code(scope),
        scope
    );

    if (!balance.lhs->getType()->isPointerTy())
        throw std::runtime_error("Cannot assign to non-pointer type.");

    const auto lhs_ptr = static_cast<llvm::AllocaInst*>(balance.lhs);

    if (lhs_ptr->getAllocatedType() != balance.rhs->getType())
        balance.rhs = attempt_cast(balance.rhs, lhs_ptr->getAllocatedType(), scope);

    return scope.builder.CreateStore(
        balance.rhs,
        balance.lhs
    );
}