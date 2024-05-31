#include "basic_codegen.h"

#include "types.h"
#include "operators.h"
#include "data.h"
#include "../ast/util.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>

using namespace ast::nodes;
using namespace cg;

llvm::LLVMContext context;

scope_data gen_inner_scope(scope_data &scope) {
    scope_data data = scope;
    data.block = llvm::BasicBlock::Create(scope.context, "entry", scope.current_function);
    data.builder.SetInsertPoint(data.block);
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

std::shared_ptr<llvm::Module> cg::generate_code(const ast::nodes::root &root) {
    auto module = std::make_shared<llvm::Module>("main", context);
    llvm::IRBuilder<> builder { context };

    scope_data prog_scope {
        context,
        module,
        builder,
        std::vector { std::make_shared<var_table>() },
        std::make_shared<std::unordered_map<std::string_view, struct_definition>>(),
        std::make_shared<std::unordered_map<std::string_view, function_definition>>(),
    };

    root.generate_code(prog_scope);
//    module->print(ostream, nullptr);

    return module;
}

llvm::Value* root::generate_code(cg::scope_data &scope) const {
    for (const auto &stmt : program_level_statements) {
        if (auto *prototype = dynamic_cast<function_prototype*>(stmt.get())) {
            if (scope.functions_table->contains(prototype->fn_name))
                throw std::runtime_error("Function already exists in symbol table.");

            scope.functions_table->emplace(prototype->fn_name, function_definition { prototype, nullptr });
        } else {
            stmt->generate_code(scope);
        }
    }

    for (const auto &[name, fn] : *scope.functions_table) {
        if (name == "main")
            fn.node->generate_code(scope);
    }

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
                stringify(std::get<std::string_view>(value))
            );
        default:
            std::unreachable();
    }
}

llvm::Value* cast::generate_code(cg::scope_data &scope) const {
    auto *expr_val = expr->generate_code(scope);
    auto *llvm_cast_type = get_llvm_type(cast_type, scope);

    auto expr_type = expr->get_type();

    if (expr_type == cast_type)
        return expr_val;

    if (expr_type.is_pointer() && cast_type.is_pointer())
        return scope.builder.CreatePointerCast(expr_val, llvm_cast_type);

    if (!expr_type.is_intrinsic() || !cast_type.is_intrinsic())
        throw std::runtime_error("Cannot cast non-intrinsic types.");

    if (expr_type.is_int() && cast_type.is_int())
        return scope.builder.CreateIntCast(expr_val, llvm_cast_type, cast_type.is_signed());

    if (!expr_type.is_int() && !cast_type.is_int())
        return scope.builder.CreateFPCast(expr_val, llvm_cast_type);

    if (expr_type.is_int() && cast_type.is_fp()) {
        if (expr_type.is_signed())
            return scope.builder.CreateSIToFP(expr_val, llvm_cast_type);
        else
            return scope.builder.CreateUIToFP(expr_val, llvm_cast_type);
    }

    if (expr_type.is_fp() && cast_type.is_int()) {
        if (cast_type.is_signed())
            return scope.builder.CreateFPToSI(expr_val, llvm_cast_type);
        else
            return scope.builder.CreateFPToUI(expr_val, llvm_cast_type);
    }

    throw std::runtime_error("Invalid cast.");
}

llvm::Value* load::generate_code(cg::scope_data &scope) const {
    auto val = expr->generate_code(scope);
    auto type = expr->get_type();

    if (type.is_var_ref && type.is_pointer())
        return val;
    else if (type.is_var_ref || type.is_pointer())
        return scope.builder.CreateLoad(get_llvm_type(type, scope), val);

    throw std::runtime_error("Cannot load non-pointer and non-argument type.");
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
    auto type = get_llvm_type(array_type.dereference(), scope);

    llvm::Value* aggregate = llvm::UndefValue::get(llvm::ArrayType::get(type, values.size()));

    for (auto i = 0; i < values.size(); ++i)
        aggregate = scope.builder.CreateInsertValue(aggregate, values[i]->generate_code(scope), i);

    return aggregate;
}

llvm::Value* initialization::generate_code(cg::scope_data &scope) const {
    auto init_type = get_type();
    llvm::Type* type;

    if (init_type.array_length == -1)
        throw std::runtime_error("Cannot initialize array with unknown length.");

    if (init_type.array_length)
        type = llvm::ArrayType::get(get_llvm_type(init_type.dereference(), scope), init_type.array_length);
    else
        type = get_llvm_type(init_type, scope);

    return add_to_table(scope.var_tables, instance.var_name, scope_variable {
        .var_allocation = scope.builder.CreateAlloca(type),
        .struct_type = get_struct_ref(instance.type, scope),
        .is_const = false
    }).var_allocation;
}

llvm::Value* method_call::generate_code(cg::scope_data &scope) const {
    llvm::Function* func = scope.functions_table->at(method_name).get(scope);

    if (!func)
        throw std::runtime_error("Function not found.");

    std::vector<llvm::Value*> args;

    for (auto &arg : arguments)
        args.emplace_back(arg->generate_code(scope));

    if (arguments.size() != func->arg_size() && !func->isVarArg())
        throw std::runtime_error("Argument count mismatch.");

    return scope.builder.CreateCall(func, args);
}

llvm::Value* var_ref::generate_code(cg::scope_data &scope) const {
    const auto &var = scope.get_variable(var_name);

    return var.var_allocation;
}

llvm::Value* return_op::generate_code(cg::scope_data &scope) const {
    if (val) {
        return scope.builder.CreateRet(
            val->generate_code(scope)
        );
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

llvm::Value *expression_root::generate_code(cg::scope_data &scope) const {
    return expr->generate_code(scope);
}

llvm::Value *for_loop::generate_code(cg::scope_data &scope) const {
    init->generate_code(scope);
    auto pre_ins_pt = scope.builder.GetInsertBlock();

    auto *loop_body = body.generate_code(scope);
    loop_body->setName("loop_body");
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

    return in_scope_block.block;
}

llvm::Function* function_prototype::generate_code(cg::scope_data &scope) const {
    llvm::Type *type = get_llvm_type(this->return_type, scope);

    // As the compiler builds functions as they are called from another method, in case we are
    // coming from a function call, the insert block needs to be saved, so we can return to it.
    auto current_block = scope.builder.GetInsertBlock();

    std::vector<llvm::Type*> llvm_parameters;

    llvm_parameters.reserve(params.data.size());
    for (const auto &param : params.data)
        llvm_parameters.emplace_back(get_llvm_type(param.instance.type, scope));

    llvm::FunctionType *func_type = llvm::FunctionType::get(type, llvm_parameters, params.is_var_args);
    llvm::Value *func_val = scope.module->getOrInsertFunction(fn_name, func_type).getCallee();
    llvm::Function *func = llvm::cast<llvm::Function>(func_val);

    for (auto i = 0; i < params.data.size(); ++i)
        func->args().begin()[i].setName(params.data[i].instance.var_name);

    scope.functions_table->at(fn_name).function = func;

    if (implementation) {
        scope.current_function = func;
        implementation->generate_code(scope);
        scope.current_function = nullptr;
    }

    scope.builder.SetInsertPoint(current_block);
    return func;
}

llvm::Value* function::generate_code(cg::scope_data &scope) const {
    if (!prototype->params.data.empty()) {
        bool block_needed = false;
        auto param_scope = gen_inner_scope(scope);

        for (size_t i = 0; i < prototype->params.data.size(); ++i) {
            auto &param = prototype->params.data[i];
            auto *arg = &scope.current_function->args().begin()[i];

            arg->setName(param.instance.var_name);

            auto type = param.get_type();

            if (type.is_pointer()) {
                add_to_table(param_scope.var_tables, param.instance.var_name, scope_variable {
                    .var_allocation = arg,
                    .struct_type = get_struct_ref(type, scope),
                    .is_const = false
                });
            } else {
                block_needed = true;
                param_scope.builder.CreateStore(
                    arg,
                    param.generate_code(param_scope)
                );
            }
        }

        auto body_scope = body.generate_code(param_scope);
        scope.builder.SetInsertPoint(param_scope.block);

        if (!block_needed)
            param_scope.block->eraseFromParent();
        else
            scope.builder.CreateBr(body_scope);
    } else {
        body.generate_code(scope);
    }

    return nullptr;
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
    auto *r_val = rhs->generate_code(scope);

    if (op) {
        r_val = scope.builder.CreateBinOp(
                get_llvm_binop(*op, r_val->getType()->isFloatingPointTy()),
                scope.builder.CreateLoad(r_val->getType(), l_val),
                r_val
        );
    }

    return scope.builder.CreateStore(r_val, l_val);
}

llvm::Function *cg::function_definition::get(scope_data &scope) {
    if (function)
        return function;

    if (!node->generate_code(scope))
        throw std::runtime_error("Function could not be generated.");

    return function;
}