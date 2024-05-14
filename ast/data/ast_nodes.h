#pragma once

#include "node_interfaces.h"

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
        CG_BASICGEN();
        ~method_call() override = default;
    };

    struct initialization : expression {
        type_instance variable;

        initialization(initialization&&) noexcept = default;
        initialization(type_instance variable, bool is_volatile = false)
            : variable(variable) {}

        void print(size_t depth) const override;
        CG_BASICGEN();
        ~initialization() override = default;
    };

    struct var_ref : expression {
        std::string_view name;

        var_ref(var_ref&&) noexcept = default;
        var_ref(std::string_view name) : name(name) {}

        void print(size_t depth) const override;
        CG_BASICGEN();
        ~var_ref() override = default;
    };

    struct raw_var : expression {
        std::string_view name;

        raw_var(raw_var&&) noexcept = default;
        raw_var(std::string_view name) : name(name) {}

        void print(size_t depth) const override;
        CG_BASICGEN();
        ~raw_var() = default;
    };

    struct un_op : expression {
        un_op_type type;
        std::unique_ptr<expression> value;

        un_op(un_op&&) noexcept = default;
        un_op(un_op_type type, std::unique_ptr<expression> value) noexcept
            : type(type), value(std::move(value)) {}

        void print(size_t depth) const override;
        CG_BASICGEN();
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
        CG_BASICGEN();
        ~bin_op() = default;
    };

    struct assignment : expression {
        std::unique_ptr<expression> lhs, rhs;
        std::optional<bin_op_type> op = std::nullopt;

        assignment(assignment&&) noexcept = default;
        assignment(std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs)
                : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        assignment(std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs, bin_op_type op)
                : lhs(std::move(lhs)), rhs(std::move(rhs)), op(std::make_optional(op)) {}

        void print(size_t depth) const override;
        CG_BASICGEN();
        ~assignment() = default;
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
        CG_BASICGEN();
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
        CG_BASICGEN();
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

        ~if_statement() override = default;
        void print(size_t else_child) const override;
        CG_BASICGEN();
    };

    struct loop : statement {
        bool pre_eval = true;
        std::unique_ptr<expression> condition;
        scope_block body;

        loop(loop&&) noexcept = default;
        loop(bool pre_eval, std::unique_ptr<expression> condition, scope_block body)
                : pre_eval(pre_eval), condition(std::move(condition)), body(std::move(body)) {}

        ~loop() override = default;
        void print(size_t depth) const override;
        CG_BASICGEN();
    };

    struct for_loop : statement {
        std::unique_ptr<expression> init;
        std::unique_ptr<expression> condition;
        std::unique_ptr<expression> update;
        scope_block body;

        for_loop(for_loop&&) noexcept = default;
        for_loop(std::unique_ptr<expression> init, std::unique_ptr<expression> condition,
                 std::unique_ptr<expression> update, scope_block body)
                : init(std::move(init)), condition(std::move(condition)),
                  update(std::move(update)), body(std::move(body)) {}

        ~for_loop() override = default;
        void print(size_t depth) const override;
        CG_BASICGEN();
    };

    struct expression_root : statement {
        std::unique_ptr<expression> expr;

        expression_root(expression_root&&) noexcept = default;
        expression_root(std::unique_ptr<expression> expr) : expr(std::move(expr)) {}

        ~expression_root() override = default;
        void print(size_t depth) const override;
        CG_BASICGEN();
    };

    // -- Root Node -----------------

    struct root : codegen_node {
        std::vector<function> functions;
        std::vector<type_instance> global_vars;

        root() noexcept = default;
        root(root&&) noexcept = default;

        ~root() override = default;
        void print(size_t depth = 0) const override;
        CG_BASICGEN();
    };
}
