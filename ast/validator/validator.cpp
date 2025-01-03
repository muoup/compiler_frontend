#include "validator.hpp"

using namespace ast;

std::unordered_map<std::string_view, nodes::function_prototype*> functions;
std::unordered_map<std::string_view, nodes::struct_declaration*> struct_names;

std::vector<std::unordered_map<std::string_view, nodes::variable_type>> scopes;

nodes::function_prototype* current_function;

void val::validate(ast::nodes::root &root) {
    functions.clear();

    scopes.clear();
    scopes.emplace_back();

    for (auto& stmt : root.program_level_statements) {
        if (auto *func = dynamic_cast<nodes::function_prototype*>(stmt.get()))
            cache_function(func);
        else if (auto *struct_decl = dynamic_cast<nodes::struct_declaration*>(stmt.get()))
            cache_struct(struct_decl);
    }

    for (auto& [_, func] : functions)
        if (func->implementation)
            validate_function(func);
}

static void val::cache_function(ast::nodes::function_prototype* func) {
    auto find = functions.find(func->fn_name);
    auto found = find != functions.end();
    auto has_implementation = found && find->second->implementation != nullptr;

    if (!found || !has_implementation)
        functions[func->fn_name] = func;
    else if (func->implementation)
        throw std::runtime_error("Function " + std::string(func->fn_name) + " already has an implementation");
}

static void val::cache_struct(ast::nodes::struct_declaration* func) {
    if (struct_names.contains(func->struct_name))
        throw std::runtime_error("Struct " + std::string(func->struct_name) + " already exists");

    struct_names[func->struct_name] = func;
}

static void val::validate_function(ast::nodes::function_prototype* func) {
    scopes.emplace_back();
    current_function = func;

    for (auto& param : func->params.data)
        scopes.back().emplace(param.instance.var_name, param.instance.type);

    validate_block(&func->implementation->body);

    current_function = nullptr;
    scopes.pop_back();
}

static void val::validate_block(ast::nodes::scope_block* block) {
    scopes.emplace_back();

    for (auto& stmt : block->statements)
        validate_statement(stmt.get());

    scopes.pop_back();
}

static void val::validate_statement(ast::nodes::statement* stmt) {
    if (!stmt)
        return;

    if (auto *return_op = dynamic_cast<ast::nodes::return_op*>(stmt)) {
        if (!return_op->val)
            return;

        validate_expression(return_op->val);
        cast_to(return_op->val, current_function->return_type);
    } else if (auto *if_stmt = dynamic_cast<ast::nodes::if_statement*>(stmt)) {
        validate_expression(if_stmt->condition);
        validate_block(&if_stmt->body);
    } else if (auto *for_loop = dynamic_cast<ast::nodes::for_loop*>(stmt)) {
        scopes.emplace_back();

        validate_expression(for_loop->init);
        validate_expression(for_loop->condition);
        validate_expression(for_loop->update);

        validate_block(&for_loop->body);

        scopes.pop_back();
    } else if (auto *loop = dynamic_cast<ast::nodes::for_loop*>(stmt)) {
        validate_expression(loop->condition);
        validate_block(&loop->body);
    } else if (auto *block = dynamic_cast<ast::nodes::scope_block*>(stmt)) {
        validate_block(block);
    } else if (auto *expr = dynamic_cast<ast::nodes::expression_root*>(stmt)) {
        validate_expression(expr->expr);
    } else if (auto *match = dynamic_cast<ast::nodes::match*>(stmt)) {
        validate_expression(match->match_expr);

        for (auto& case_ : match->cases) {
            cast_to(case_.match_expr, match->match_expr->get_type());
            validate_block(&case_.body);
        }
    }
}

static void val::validate_expression(std::unique_ptr<ast::nodes::expression> &tree) {
    if (!tree)
        return;

    if (auto *call = dynamic_cast<ast::nodes::method_call *>(tree.get())) {
        validate_method_call(call);
    } else if (auto *b_op = dynamic_cast<ast::nodes::bin_op *>(tree.get())) {
        validate_bin_op(b_op);
    } else if (auto *u_op = dynamic_cast<ast::nodes::un_op*>(tree.get())) {
        validate_un_op(u_op);
    } else if (auto *assn = dynamic_cast<ast::nodes::assignment*>(tree.get())) {
        validate_assn(assn);
    } else if (auto *expr = dynamic_cast<ast::nodes::expression_root*>(tree.get())) {
        validate_expression(expr->expr);
    } else if (auto *init = dynamic_cast<ast::nodes::initialization*>(tree.get())) {
        scopes.back().emplace(init->instance.var_name, init->instance.type);
    }
}

static void val::validate_method_call(ast::nodes::method_call *call) {
    auto find = functions.find(call->method_name);

    if (find == functions.end())
        throw std::runtime_error("Function " + std::string(call->method_name) + " not found");

    auto *func = find->second;

    if (call->arguments.size() < func->params.data.size())
        throw std::runtime_error("Too few arguments for function " + std::string(call->method_name));

    for (size_t i = 0; i < func->params.data.size(); ++i) {
        validate_expression(call->arguments[i]);
        cast_to(call->arguments[i], func->params.data[i].instance.type);
    }

    if (call->arguments.size() > func->params.data.size()) {
        if (!func->params.is_var_args)
            throw std::runtime_error("Too many arguments for function " + std::string(call->method_name));

        for (size_t i = func->params.data.size(); i < call->arguments.size(); ++i) {
            validate_expression(call->arguments[i]);

            if (!call->arguments[i]->get_type().is_pointer())
                cast_to(call->arguments[i], nodes::variable_type{nodes::intrinsic_type::i32});
        }
    }

    call->return_type = func->return_type;
}

static void val::validate_bin_op(ast::nodes::bin_op *op) {
    validate_expression(op->left);
    validate_expression(op->right);

    auto l_type = op->left->get_type();
    auto r_type = op->right->get_type();

    if (op->type == nodes::bin_op_type::acc) {
        validate_access(op);
        return;
    }

    if (l_type.is_intrinsic() && r_type.is_intrinsic()
     || l_type.is_pointer() && r_type.is_pointer()) {
        if (l_type == r_type)
            return;

        cast_to(op->right, l_type);
        return;
    }

    throw std::runtime_error("Cannot perform operation on non-intrinsic types");
}

static void val::validate_un_op(ast::nodes::un_op *op) {
    if (op->type == ast::nodes::un_op_type::addr_of)
        return;

    validate_expression(op->value);

    switch (op->type) {
        case ast::nodes::un_op_type::deref:
            create_load(op->value);
            break;
        case ast::nodes::un_op_type::negate:
            op->value = std::make_unique<ast::nodes::bin_op>(
                    ast::nodes::bin_op_type::sub,
                    std::make_unique<ast::nodes::literal>(0),
                    std::move(op->value)
            );
            break;
        case ast::nodes::un_op_type::bit_not:
            op->value = std::make_unique<ast::nodes::bin_op>(
                    ast::nodes::bin_op_type::b_xor,
                    std::make_unique<ast::nodes::literal>(-1),
                    std::move(op->value)
            );
            break;
        case ast::nodes::un_op_type::log_not:
            op->value = std::make_unique<ast::nodes::bin_op>(
                    ast::nodes::bin_op_type::eq,
                    std::make_unique<ast::nodes::literal>(0),
                    std::move(op->value)
            );
            break;
        default:
            throw std::runtime_error("Invalid unary operation");
    }
}

static void val::validate_assn(ast::nodes::assignment *assn) {
    validate_expression(assn->lhs);
    validate_expression(assn->rhs);

    if (auto *load = dynamic_cast<ast::nodes::load*>(assn->lhs.get()))
        assn->lhs = std::move(load->expr);

    cast_to(assn->rhs, assn->lhs->get_type());

    if (auto *init = dynamic_cast<ast::nodes::initialization*>(assn->lhs.get())) {
        init->instance.type.array_length = assn->rhs->get_type().array_length;
    }
}

static void val::validate_access(ast::nodes::bin_op *access) {
    validate_expression(access->left);

    auto l_type = access->left->get_type();
    auto r_access = dynamic_cast<ast::nodes::var_ref*>(access->right.get());

    if (l_type.is_pointer()) {
        cast_to(access->right, nodes::variable_type{ nodes::intrinsic_type::i64 });
        return;
    }

    if (!r_access)
        throw std::runtime_error("Right side of access is not a field (var_ref)");

    auto struct_name = std::get<std::string_view>(l_type.type);

    if (!struct_names.contains(struct_name))
        throw std::runtime_error("Struct " + std::string(struct_name) + " not found");

    auto *struct_decl = struct_names.at(struct_name);

    for (auto& field : struct_decl->fields) {
        if (field.var_name == r_access->var_name) {
            r_access->type = field.type;
            return;
        }
    }

    throw std::runtime_error("Field " + std::string(r_access->var_name) + " not found in struct " + std::string(struct_name));
}

static void val::cast_to(std::unique_ptr<ast::nodes::expression> &expr, ast::nodes::variable_type type) {
    auto pre_type = expr->get_type();

    if (pre_type == type)
        return;

    auto initializer = dynamic_cast<ast::nodes::initializer_list*>(expr.get());

    if (initializer) {
        if (initializer->struct_hint != "") {
            if (type.is_intrinsic() || std::get<std::string_view>(type.type) != initializer->struct_hint)
                throw std::runtime_error("Struct initializer does not match type hint");

            expr = std::make_unique<ast::nodes::struct_initializer>(
                    struct_names.at(initializer->struct_hint),
                    std::move(initializer->values)
            );
        } else if (!type.is_intrinsic()) {
            expr = std::make_unique<ast::nodes::struct_initializer>(
                    struct_names.at(std::get<std::string_view>(type.type)),
                    std::move(initializer->values)
            );
        } else if (type.is_pointer()) {
            type.array_length = initializer->values.size();
            expr = std::make_unique<ast::nodes::array_initializer>(
                type,
                std::move(initializer->values)
            );
        } else {
            throw std::runtime_error("Cannot cast initializer list to a primitive type");
        }

        return;
    }

    if (!type.is_intrinsic() && !type.is_pointer())
        throw std::runtime_error("Cannot cast to non-intrinsic type");

    expr = std::make_unique<ast::nodes::cast>(std::move(expr), type);
}
static void val::create_load(std::unique_ptr<ast::nodes::expression> &expr) {
    auto type = expr->get_type();

    if (!type.is_pointer() && !type.is_var_ref)
        throw std::runtime_error("Cannot load non-pointer type");

    expr = std::make_unique<ast::nodes::load>(std::move(expr));
}

static std::optional<nodes::variable_type> val::find_variable(std::string_view name) {
    for (auto iter = scopes.rbegin(); iter != scopes.rend(); ++iter) {
        auto find = iter->find(name);

        if (find != iter->end())
            return find->second;
    }

    return std::nullopt;
}
