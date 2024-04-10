#pragma once

#include "node_interfaces.h"
#include "LLVM/IR/Value.h"

namespace ast::nodes {

    // -- Expression Nodes ----------

    struct method_call : expression {
        std::string_view method_name;
        std::vector<std::unique_ptr<expression>> arguments;

        method_call(method_call&&) noexcept = default;
        method_call(std::string_view method_name,
                    std::vector<std::unique_ptr<expression>> arguments)
                : method_name(method_name), arguments(std::move(arguments)) {}

        void print(size_t depth) const override;
        CODEGEN() override;
        ~method_call() = default;
    };

    struct initialization : expression {
        type_instance variable;

        initialization(initialization&&) noexcept = default;
        initialization(type_instance variable, bool is_volatile = false)
            : variable(variable) {}

        void print(size_t depth) const override;
        CODEGEN() override;
        ~initialization() = default;
    };

    struct var_ref : expression {
        std::string_view name;

        var_ref(var_ref&&) noexcept = default;
        var_ref(std::string_view name) : name(name) {}

        void print(size_t depth) const override;
        CODEGEN() override;
        ~var_ref() = default;
    };

    struct raw_var : expression {
        std::string_view name;

        raw_var(raw_var&&) noexcept = default;
        raw_var(std::string_view name) : name(name) {}

        void print(size_t depth) const override;
        CODEGEN() override;
        ~raw_var() = default;
    };

    struct assignment : expression {
        std::unique_ptr<expression> lhs, rhs;
        assn_type type;

        assignment(assignment&&) noexcept = default;
        assignment(std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs, assn_type type)
                : lhs(std::move(lhs)), rhs(std::move(rhs)), type(type) {}

        void print(size_t depth) const override;
        CODEGEN() override;
        ~assignment() = default;
    };

    struct un_op : expression {
        un_op_type type;
        std::unique_ptr<expression> value;

        un_op(un_op&&) noexcept = default;
        un_op(un_op_type type, std::unique_ptr<expression> value) noexcept
            : type(type), value(std::move(value)) {}

        void print(size_t depth) const override;
        CODEGEN() override;
        ~un_op() = default;
    };

    struct bin_op : expression {
        bin_op_type type;
        std::unique_ptr<expression> left;
        std::unique_ptr<expression> right;

        bin_op(bin_op&&) noexcept = default;
        bin_op(bin_op_type type, std::unique_ptr<expression> left, std::unique_ptr<expression> right) noexcept
                : type(type), left(std::move(left)), right(std::move(right)) {}

        void print(size_t depth) const override;
        CODEGEN() override;
        ~bin_op() = default;
    };

    enum literal_type {
        UINT,
        INT,
        FLOAT,
        CHAR,
        STRING
    };
    struct literal : expression {
        using lit_variant = std::variant<unsigned int, int, double, char, std::string_view>;

        lit_variant value;
        uint8_t type_size = 32;

        literal(literal&&) noexcept = default;
        literal(lit_variant value, uint8_t type_size = 32) noexcept : value(std::move(value)), type_size(type_size) {}

        void print(size_t depth) const override;
        CODEGEN() override;
        ~literal() = default;
    };

    // -- Statement Nodes ----------

    struct return_op : statement {
        std::unique_ptr<expression> val;

        return_op() = default;
        return_op(return_op&&) noexcept = default;
        return_op(std::unique_ptr<expression> val) : val(std::move(val)) {}

        void print(size_t depth) const override;
        ~return_op() = default;
        CODEGEN() override;
    };

    struct if_statement : statement {
        std::unique_ptr<expression> condition;
        scope_block body;
        std::optional<scope_block> else_body = std::nullopt;

        if_statement(if_statement&&) noexcept = default;
        if_statement(std::unique_ptr<expression> condition, scope_block body)
            : condition(std::move(condition)), body(std::move(body)) {}
        if_statement(std::unique_ptr<expression> condition, scope_block body,
                     std::optional<scope_block> else_body)
            : condition(std::move(condition)), body(std::move(body)), else_body(std::move(else_body)) {}

        ~if_statement() = default;
        void print(size_t else_child) const override;
        CODEGEN() override;
    };

    struct loop : statement {
        bool pre_eval = true;
        std::unique_ptr<expression> condition;
        scope_block body;

        loop(loop&&) noexcept = default;
        loop(bool pre_eval, std::unique_ptr<expression> condition, scope_block body)
                : pre_eval(pre_eval), condition(std::move(condition)), body(std::move(body)) {}

        ~loop() = default;
        void print(size_t depth) const override;
        CODEGEN() override;
    };

    struct expression_root : statement {
        std::unique_ptr<expression> expr;

        expression_root(expression_root&&) noexcept = default;
        expression_root(std::unique_ptr<expression> expr) : expr(std::move(expr)) {}

        ~expression_root() = default;
        inline void print(size_t depth) const override {
            expr->print(depth);
        };
        inline CODEGEN() override {
            return expr->generate_code(scope);
        };
    };

    // -- Root Node -----------------

    struct root : codegen_node {
        std::vector<function> functions;
        std::vector<type_instance> global_vars;

        root() noexcept = default;
        root(root&&) noexcept = default;
        ~root() = default;

        void print(size_t depth = 0) const override;
        CODEGEN() override;
    };
}
