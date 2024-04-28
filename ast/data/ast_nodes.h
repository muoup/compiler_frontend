#pragma once

#include "node_interfaces.h"
#include "LLVM/IR/Value.h"

namespace ast::nodes {
    // -- Expression Nodes ----------

    /**
     *  Code Block: Non-Abstract Data Type
     *  ----------------------------------
     *  A code block represents a scope, any code blocks
     *  within will have their own innermost scope but will
     *  also be able to reference any outer scopes.
     */
    struct scope_block : expression {
        using scope_stmts = std::vector<std::unique_ptr<statement>>;
        scope_stmts statements;

        scope_block() = default;
        scope_block(scope_block&&) noexcept = default;
        scope_block(scope_stmts statements)
                : statements(std::move(statements)) {}

        ~scope_block() = default;
        void print(size_t depth) const override;
        llvm::BasicBlock* generate_code(cg::scope_data &scope) const override;

        value_type get_type() const override;
    };

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

        value_type get_type() const override;
    };

    struct initialization : expression {
        type_instance variable;

        initialization(initialization&&) noexcept = default;
        initialization(type_instance variable, bool is_volatile = false)
            : variable(variable) {}

        void print(size_t depth) const override;
        CODEGEN() override;
        ~initialization() = default;

        value_type get_type() const override;
    };

    struct var_ref : expression {
        std::string_view name;
        std::optional<nodes::value_type> type = std::nullopt;

        var_ref(var_ref&&) noexcept = default;
        var_ref(std::string_view name, std::optional<nodes::value_type> type) : name(name), type(type) {}

        void print(size_t depth) const override;
        CODEGEN() override;
        ~var_ref() = default;

        value_type get_type() const override;
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

        value_type get_type() const override;
    };

    struct bin_op : expression {
        bin_op_type type;
        std::unique_ptr<expression> left;
        std::unique_ptr<expression> right;

        bin_op(bin_op&&) noexcept = default;
        bin_op(bin_op_type type, std::unique_ptr<expression> left, std::unique_ptr<expression> right) noexcept
                : type(type), left(std::move(left)), right(std::move(right)) {}

        virtual void populate(std::unique_ptr<expression> left, std::unique_ptr<expression> right) {
            this->left = std::move(left);
            this->right = std::move(right);
        }

        void print(size_t depth) const override;
        CODEGEN() override;
        ~bin_op() = default;

        value_type get_type() const override;
    };

    struct match : expression {
        struct match_case {
            std::unique_ptr<expression> match_expr;
            scope_block body;
        };

        std::unique_ptr<expression> match_expr;
        std::vector<match_case> cases;
        std::unique_ptr<scope_block> default_case;

        match(match&&) noexcept = default;
        match() = default;
        match(std::unique_ptr<expression> match_expr, std::vector<match_case> cases)
                : match_expr(std::move(match_expr)), cases(std::move(cases)) {}

        ~match() = default;
        void print(size_t depth) const override;
        CODEGEN() override;

        value_type get_type() const override;
    };

//    struct accessor : bin_op {
//        bool is_arrow;
//        std::unique_ptr<expression> left;
//        std::unique_ptr<expression> right;
//
//        accessor(accessor&&) noexcept = default;
//        accessor(bool is_arrow, std::unique_ptr<expression> left, std::unique_ptr<expression> right) noexcept
//                : is_arrow(is_arrow), left(std::move(left)), right(std::move(right)) {}
//
//        void print(size_t depth) const override;
//        CODEGEN() override;
//        ~accessor() = default;
//
//        void populate(std::unique_ptr<expression> left, std::unique_ptr<expression> right) override {
//            this->left = std::move(left);
//            this->right = std::move(right);
//        }
//
//        value_type get_type() const override;
//    };

    struct assignment : expression {
        std::unique_ptr<expression> lhs, rhs;
        std::optional<bin_op_type> op = std::nullopt;

        assignment(assignment&&) noexcept = default;
        assignment(std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs)
                : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        assignment(std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs, std::optional<bin_op_type> op)
                : lhs(std::move(lhs)), rhs(std::move(rhs)), op(op) {}

        void print(size_t depth) const override;
        CODEGEN() override;
        ~assignment() = default;

        value_type get_type() const override;
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

        value_type get_type() const override;
    };

    struct cast : expression {
        std::unique_ptr<expression> expr;
        value_type cast_type;

        cast(cast&&) noexcept = default;
        cast(std::unique_ptr<expression> expr, value_type cast_type)
            : expr(std::move(expr)), cast_type(cast_type) {}

        void print(size_t depth) const override;
        CODEGEN() override;

        value_type get_type() const override;
    };

    struct load : expression {
        std::unique_ptr<expression> expr;

        load(load&&) noexcept = default;
        load(std::unique_ptr<expression> expr) : expr(std::move(expr)) {}

        void print(size_t depth) const override;
        CODEGEN() override;

        value_type get_type() const override;
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

        ~for_loop() = default;
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

    // -- Program Level Nodes -------

    /**
     *  Function: Non-Abstract Data Type
     *  -------------------------------
     *  A function represents a callable block of code at the top level of the program.
     *  Functionally this acts very similar to a statement, but differs in how it must
     *  be implemented into a program. In the future, if functions are able to be implemented
     *  within a function, the line between a function and a statement will blur. But for now,
     *  functions are a top-level construct.
     */
    struct function : program_level_stmt {
        value_type return_type;
        std::string_view fn_name;
        std::vector<type_instance> param_types;
        scope_block body;

        function(function&&) noexcept = default;
        function(value_type return_type, std::string_view method_name, std::vector<type_instance> param_types, scope_block body)
                : return_type(return_type), fn_name(method_name), param_types(std::move(param_types)), body(std::move(body)) {}

        ~function() = default;
        void print(size_t depth) const override;
        CODEGEN() override;
    };

    /**
     *  Struct Declaration: Non-Abstract Data Type
     *  ------------------------------------------
     *  A struct defines a new type, containing a list of fields.
     *  This is used to define new types in the program.
     */
    struct struct_declaration : program_level_stmt {
        std::string_view name;
        std::vector<type_instance> fields;

        struct_declaration(struct_declaration&&) noexcept = default;
        struct_declaration(std::string_view name, std::vector<type_instance> fields)
                : name(name), fields(std::move(fields)) {}

        ~struct_declaration() = default;
        void print(size_t depth) const override;
        CODEGEN() override;
    };

    // -- Root Node -----------------

    struct root : codegen_node {
        std::vector<std::unique_ptr<program_level_stmt>> program_level_statements;

        root() noexcept = default;
        root(root&&) noexcept = default;
        ~root() = default;

        void print(size_t depth = 0) const override;
        CODEGEN() override;
    };
}
