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
    data.var_tables.emplace_back(std::make_shared<var_table>());
    return data;
}

template <typename T>
using generic_table = std::vector<std::shared_ptr<std::unordered_map<std::string_view, T>>>;

template <typename T>
const T& add_to_table(const generic_table<T> &table, std::string_view name, T value) {
    if (table.back()->contains(name))
        throw std::runtime_error("Variable already exists in symbol table.");

    table.back()->emplace(name, value);
    return table.back()->at(name);
}

template <typename T>
const T& get_from_table(const generic_table<T> &table, std::string_view name) {
    for (auto it = table.rbegin(); it != table.rend(); ++it) {
        if ((*it)->contains(name))
            return (*it)->at(name);
    }

    throw std::runtime_error("Index could not be found in table.");
}

const scope_variable& scope_data::get_variable(std::string_view name) const {
    return get_from_table(var_tables, name);
}

const struct_definition& scope_data::get_struct(std::string_view name) const {
    return struct_table->at(name);
}

void cg::generate_code(const ast::nodes::root &root, llvm::raw_ostream &ostream) {
    llvm::LLVMContext context;
    auto module = std::make_shared<llvm::Module>("main", context);
    llvm::IRBuilder<> builder { context };

    scope_data scope {
        context,
        module,
        builder,
        std::vector { std::make_shared<var_table>() },
        std::make_shared<std::unordered_map<std::string_view, struct_definition>>()
    };

    root.generate_code(scope);
    module->print(ostream, nullptr);
}

llvm::Value* root::generate_code(cg::scope_data &scope) const {
    for (const auto &prog_stmts : program_level_statements)
        prog_stmts->generate_code(scope);

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

llvm::Value* cast::generate_code(cg::scope_data &scope) const {
    auto *expr_val = expr->generate_code(scope);
    auto *llvm_cast_type = get_llvm_type(this->cast_type, scope);

    return attempt_cast(expr_val, llvm_cast_type, scope);
}

llvm::Value* load::generate_code(cg::scope_data &scope) const {
    auto *val = expr->generate_code(scope);

    if (!val->getType()->isPointerTy())
        throw std::runtime_error("Cannot dereference non-pointer type.");

    auto *new_type = get_llvm_type(get_type(), scope);

    return scope.builder.CreateLoad(new_type, val);
}

llvm::Value* expression_shield::generate_code(cg::scope_data &scope) const {
    return expr->generate_code(scope);
}

llvm::Value* initializer_list::generate_code(cg::scope_data &scope) const {
    throw std::runtime_error("Initializer lists are not yet implemented.");
}

llvm::Value* struct_initializer::generate_code(cg::scope_data &scope) const {
    auto struct_def = scope.get_struct(struct_type);
    auto struct_size = struct_def.field_decls.size();

    llvm::Value* aggregate = llvm::UndefValue::get(struct_def.struct_type);

    for (auto i = 0; i < struct_size; i++)
        aggregate = scope.builder.CreateInsertValue(aggregate, values[i]->generate_code(scope), i);

    return aggregate;
}

llvm::Value* array_initializer::generate_code(cg::scope_data &scope) const {
    auto type = get_llvm_type(array_type, scope);

    llvm::Value* aggregate = llvm::UndefValue::get(llvm::ArrayType::get(type, values.size()));

    for (auto i = 0; i < values.size(); ++i)
        aggregate = scope.builder.CreateInsertValue(aggregate, values[i]->generate_code(scope), i);

    return aggregate;
}

llvm::Value* initialization::generate_code(cg::scope_data &scope) const {
    auto init_type = get_type();
    auto type = get_llvm_type(init_type, scope);

    if (init_type.array_length == -1)
        throw std::runtime_error("Cannot initialize array with unknown length.");

    if (init_type.array_length)
        type = llvm::ArrayType::get(type, init_type.array_length);

    return add_to_table(scope.var_tables, variable.var_name, scope_variable {
        .var_allocation = scope.builder.CreateAlloca(type),
        .struct_type = get_struct_ref(variable.type, scope),
        .is_const = false
    }).var_allocation;
}

llvm::Value* method_call::generate_code(cg::scope_data &scope) const {
    llvm::Function* func = method_name.starts_with(libc_prefix) ?
                           get_libc_fn(method_name, scope) :
                           scope.module->getFunction(method_name);

    if (!func)
        throw std::runtime_error("Function not found.");

    std::vector<llvm::Value*> args;

    for (auto i = 0; i < arguments.size(); ++i) {
        auto param_val = arguments[i]->generate_code(scope);

        if (i < func->arg_size())
            param_val = attempt_cast(param_val, func->getArg(i)->getType(), scope);
        else if (i >= func->arg_size() && !func->isVarArg())
            throw std::runtime_error("Argument count mismatch.");
        else
            param_val = varargs_cast(param_val, scope);

        args.emplace_back(param_val);
    }

    if (arguments.size() != func->arg_size() && !func->isVarArg())
        throw std::runtime_error("Argument count mismatch.");

    return scope.builder.CreateCall(func, args);
}

llvm::Value* var_ref::generate_code(cg::scope_data &scope) const {
    return scope.get_variable(var_name).var_allocation;
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

llvm::Value* match::generate_code(cg::scope_data &scope) const {
    auto switch_start = llvm::BasicBlock::Create(scope.context, "switch_start", scope.current_function);

    std::vector<llvm::BasicBlock*> cond_blocks;
    cond_blocks.reserve(cases.size());

    for (const auto &case_ : cases) {
        cond_blocks.emplace_back(case_.body.generate_code(scope));
    }

    auto *default_block = default_case ?
                          default_case->generate_code(scope) :
                          nullptr;

    auto merge_block = llvm::BasicBlock::Create(scope.context, "merge", scope.current_function);

    for (auto *block : cond_blocks) {
        scope.builder.SetInsertPoint(block);
        scope.builder.CreateBr(merge_block);
    }

    if (default_block) {
        scope.builder.SetInsertPoint(default_block);
        scope.builder.CreateBr(merge_block);
    }

    scope.builder.SetInsertPoint(switch_start);

    default_block = default_block ? default_block : merge_block;

    auto *cmp = match_expr->generate_code(scope);
    auto *switch_inst = scope.builder.CreateSwitch(cmp, default_block, cases.size());

    for (auto i = 0; i < cases.size(); ++i) {
        auto *case_ = cases[i].match_expr->generate_code(scope);
        auto *case_block = cond_blocks[i];
        auto *case_const = llvm::dyn_cast<llvm::ConstantInt>(case_);

        if (!case_const)
            throw std::runtime_error("Match case must be a constant integer.");
        if (!case_block)
            throw std::runtime_error("Match case block must be a valid block.");

        switch_inst->addCase(
                case_const,
                case_block
        );
    }

    scope.builder.CreateBr(default_block);

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

    scope.builder.SetInsertPoint(pre_init);
    scope.builder.CreateBr(pre_init ? cond_block : block);
    scope.builder.SetInsertPoint(cond_block);

    scope.builder.SetInsertPoint(ret_block);

    return nullptr;
}


llvm::Value *for_loop::generate_code(cg::scope_data &scope) const {
    init->generate_code(scope);
    auto pre_ins_pt = scope.builder.GetInsertBlock();

    auto *loop_body = body.generate_code(scope);
    auto *loop_iter = llvm::BasicBlock::Create(scope.context, "loop_iter", scope.current_function);
    auto *loop_cond = llvm::BasicBlock::Create(scope.context, "loop_cond", scope.current_function);
    auto *loop_merge = llvm::BasicBlock::Create(scope.context, "loop_merge", scope.current_function);

    scope.builder.SetInsertPoint(pre_ins_pt);
    scope.builder.CreateBr(loop_cond);

    scope.builder.SetInsertPoint(loop_body);
    scope.builder.CreateBr(loop_iter);

    scope.builder.SetInsertPoint(loop_iter);
    update->generate_code(scope);
    scope.builder.CreateBr(loop_cond);

    scope.builder.SetInsertPoint(loop_cond);
    scope.builder.CreateCondBr(
            conditional_expression(condition, scope),
            loop_body,
            loop_merge
    );

    scope.builder.SetInsertPoint(loop_merge);

    return nullptr;
}

llvm::BasicBlock* scope_block::generate_code(cg::scope_data &scope) const {
    auto in_scope_block = gen_inner_scope(scope);

    for (const auto &stmt : statements)
        stmt->generate_code(in_scope_block);

    return in_scope_block.entry;
}

llvm::Value* function::generate_code(cg::scope_data &scope) const {
    llvm::Type *type = get_llvm_type(this->return_type, scope);

    std::vector<llvm::Type*> params;

    params.reserve(param_types.size());
    for (const auto &param : param_types)
        params.emplace_back(get_llvm_type(param.type, scope));

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

llvm::Value *struct_declaration::generate_code(cg::scope_data &scope) const {
    std::vector<llvm::Type*> field_llvm_types;
    std::vector<ast::nodes::type_instance> field_decls;

    field_llvm_types.reserve(this->fields.size());
    for (const auto &field : this->fields) {
        field_llvm_types.emplace_back(get_llvm_type(field.type, scope));
        field_decls.emplace_back(field.type, field.var_name);
    }

    if (scope.struct_table->contains(struct_name))
        throw std::runtime_error("Struct already exists in symbol table.");

    scope.struct_table->emplace(struct_name, struct_definition {
        llvm::StructType::create(scope.context, field_llvm_types, struct_name),
        std::move(field_decls)
    });

    return nullptr;
}

llvm::Value* un_op::generate_code(cg::scope_data &scope) const {
    auto *val = value->generate_code(scope);

    switch (type) {
        using namespace ast::nodes;
        case un_op_type::log_not:
            return scope.builder.CreateNot(val);
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

llvm::Value* bin_op::generate_code(cg::scope_data &scope) const {
    return cg::generate_bin_op(left, right, type, scope);
}

llvm::Value* assignment::generate_code(cg::scope_data &scope) const {
    auto *l_val = lhs->generate_code(scope);
    auto *r_val = op ?
                    cg::generate_bin_op(lhs, rhs, *op, scope) :
                    rhs->generate_code(scope);

    return scope.builder.CreateStore(r_val, l_val);
}