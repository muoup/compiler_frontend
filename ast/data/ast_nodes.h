#pragma once

#include "abstract_data.h"
#include "LLVM/IR/Value.h"
#include "data_maps.h"

namespace ast::nodes {
    // -- Expression Nodes ----------

    struct method_call : expression {
        NODENAME("METHOD_CALL");
        CHILDREN(arguments);

        DETAILS(method_name);

        std::string_view method_name;
        std::vector<std::unique_ptr<expression>> arguments;

        method_call(method_call&&) noexcept = default;
        method_call(std::string_view method_name,
                    std::vector<std::unique_ptr<expression>> arguments)
                : method_name(method_name), arguments(std::move(arguments)) {}

        CODEGEN() override;
        ~method_call() = default;

        variable_type get_type() const override;
    };

    struct initialization : expression {
        NODENAME("INITIALIZATION");
        CHILDREN(variable);

        type_instance variable;

        initialization(initialization&&) noexcept = default;
        initialization(type_instance variable)
            : variable(std::move(variable)) {}

        CODEGEN() override;
        ~initialization() = default;

        variable_type get_type() const override;
    };

    struct var_ref : expression {
        NODENAME("VAR_REF");
        DETAILS(var_name, type);

        std::string_view var_name;
        std::optional<nodes::variable_type> type = std::nullopt;

        var_ref(var_ref&&) noexcept = default;
        var_ref(std::string_view name, std::optional<nodes::variable_type> type) : var_name(name), type(type) {}

        CODEGEN() override;
        ~var_ref() = default;

        variable_type get_type() const override;
    };

    struct un_op : expression {
        NODENAME("UN_OP");
        CHILDREN(value);

        DETAILS(ast::pm::find_key(ast::pm::unop_type_map, type));

        un_op_type type;
        std::unique_ptr<expression> value;

        un_op(un_op&&) noexcept = default;
        un_op(un_op_type type, std::unique_ptr<expression> value) noexcept
            : type(type), value(std::move(value)) {}

        CODEGEN() override;
        ~un_op() = default;

        variable_type get_type() const override;
    };

    struct bin_op : expression {
        NODENAME("BIN_OP");
        CHILDREN(left, right);

        DETAILS(ast::pm::find_key(ast::pm::binop_type_map, type));

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

        CODEGEN() override;
        ~bin_op() = default;

        variable_type get_type() const override;
    };

    struct match_case : printable {
        NODENAME("MATCH_CASE");
        CHILDREN(match_expr, body);

        std::unique_ptr<expression> match_expr;
        scope_block body;

        match_case(match_case&&) noexcept = default;
        match_case(std::unique_ptr<expression> match_expr, scope_block body)
                : match_expr(std::move(match_expr)), body(std::move(body)) {}
    };

    struct match : expression {
        NODENAME("MATCH");
        CHILDREN(match_expr, cases, default_case);

        std::unique_ptr<expression> match_expr;
        std::vector<match_case> cases;
        std::unique_ptr<scope_block> default_case;

        match(match&&) noexcept = default;
        match() = default;
        match(std::unique_ptr<expression> match_expr, std::vector<match_case> cases)
                : match_expr(std::move(match_expr)), cases(std::move(cases)) {}

        ~match() = default;
        CODEGEN() override;

        variable_type get_type() const override;
    };

    struct assignment : expression {
        NODENAME("ASSIGNMENT");
        CHILDREN(lhs, rhs);
        DETAILS(op ? ast::pm::find_key(ast::pm::binop_type_map, *op) : std::nullopt);

        std::unique_ptr<expression> lhs, rhs;
        std::optional<bin_op_type> op = std::nullopt;

        assignment(assignment&&) noexcept = default;
        assignment(std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs, std::optional<bin_op_type> additional_operator = std::nullopt)
                : lhs(std::move(lhs)), rhs(std::move(rhs)), op(additional_operator) {}

        CODEGEN() override;
        ~assignment() = default;

        variable_type get_type() const override;
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

        NODENAME("LITERAL");
        DETAILS(get_type_name());

        lit_variant value;
        uint8_t type_size = 32;

        literal(literal&&) noexcept = default;
        literal(lit_variant value, uint8_t type_size = 32) noexcept : value(std::move(value)), type_size(type_size) {}

        CODEGEN() override;
        ~literal() = default;

        variable_type get_type() const override;

        std::string get_type_name() const {
            switch (value.index()) {
                case UINT:
                    return "uint " + std::to_string(type_size);
                case INT:
                    return "int " + std::to_string(type_size);
                case FLOAT:
                    return "float " + std::to_string(type_size);
                case CHAR:
                    return "char " + std::to_string(type_size);
                case STRING:
                    return "string " + std::string(std::get<std::string_view>(value));
                default:
                    std::unreachable();
            }
        }
    };

    struct cast : expression {
        NODENAME("CAST");
        CHILDREN(expr);
        DETAILS(cast_type);

        std::unique_ptr<expression> expr;
        variable_type cast_type;

        cast(cast&&) noexcept = default;
        cast(std::unique_ptr<expression> expr, variable_type cast_type)
            : expr(std::move(expr)), cast_type(cast_type) {}

        CODEGEN() override;

        variable_type get_type() const override;
    };

    struct load : expression {
        NODENAME("LOAD");
        CHILDREN(expr);

        std::unique_ptr<expression> expr;

        load(load&&) noexcept = default;
        load(std::unique_ptr<expression> expr) : expr(std::move(expr)) {}

        CODEGEN() override;

        variable_type get_type() const override;
    };

    struct expression_shield : expression {
        NODENAME("EXPRESSION_SHIELD");
        CHILDREN(expr);

        std::unique_ptr<expression> expr;

        expression_shield(expression_shield&&) noexcept = default;
        expression_shield(std::unique_ptr<expression> expr) : expr(std::move(expr)) {}

        CODEGEN() override;

        variable_type get_type() const override {
            return expr->get_type();
        };
    };

    struct initializer_list : expression {
        NODENAME("INITIALIZER_LIST");
        CHILDREN(values);

        std::vector<std::unique_ptr<expression>> values;

        initializer_list(initializer_list&&) noexcept = default;
        initializer_list(std::vector<std::unique_ptr<expression>> values) : values(std::move(values)) {}

        CODEGEN() override;

        variable_type get_type() const override;
    };

    struct array_initializer : expression {
        NODENAME("ARRAY_INITIALIZER");
        CHILDREN(values);

        variable_type array_type;
        std::vector<std::unique_ptr<expression>> values;

        array_initializer(variable_type, initializer_list&&);

        CODEGEN() override;

        variable_type get_type() const override {
            return array_type;
        }
    };

    struct struct_initializer : expression {
        NODENAME("STRUCT_INITIALIZER");
        CHILDREN(values);

        std::string_view struct_type;
        std::vector<std::unique_ptr<expression>> values;

        struct_initializer(std::string_view struct_type, std::vector<std::unique_ptr<nodes::expression>> init_list);

        CODEGEN() override;

        variable_type get_type() const override {
            return { struct_type };
        }
    };

    // -- Statement Nodes ----------

    struct return_op : statement {
        NODENAME("RETURN_OP");
        CHILDREN(val);

        std::unique_ptr<expression> val;

        return_op() = default;
        return_op(return_op&&) noexcept = default;
        return_op(std::unique_ptr<expression> val) : val(std::move(val)) {}

        ~return_op() = default;
        CODEGEN() override;
    };

    struct if_statement : statement {
        NODENAME("IF_STATEMENT");
        CHILDREN(condition, body, else_body);

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
        CODEGEN() override;
    };

    struct loop : statement {
        NODENAME("LOOP");
        CHILDREN(condition, body);

        bool pre_eval = true;
        std::unique_ptr<expression> condition;
        scope_block body;

        loop(loop&&) noexcept = default;
        loop(bool pre_eval, std::unique_ptr<expression> condition, scope_block body)
                : pre_eval(pre_eval), condition(std::move(condition)), body(std::move(body)) {}

        ~loop() = default;
        CODEGEN() override;
    };

    struct for_loop : statement {
        NODENAME("FOR_LOOP");
        CHILDREN(init, condition, update, body);

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
        CODEGEN() override;
    };

    struct expression_root : statement {
        NODENAME("EXPRESSION_ROOT");
        CHILDREN(expr);

        std::unique_ptr<expression> expr;

        expression_root(expression_root&&) noexcept = default;
        expression_root(std::unique_ptr<expression> expr) : expr(std::move(expr)) {}

        ~expression_root() = default;
        inline CODEGEN() override {
            return expr->generate_code(scope);
        };
    };

    // -- Program Level Nodes -------

    struct function_prototype : program_level_stmt {
        NODENAME("FUNCTION_PROTOTYPE");
        CHILDREN(params.data);
        DETAILS(fn_name, return_type, params.is_var_args);

        variable_type return_type;
        std::string_view fn_name;
        method_params params;

        function_prototype(function_prototype&&) noexcept = default;
        function_prototype(variable_type return_type, std::string_view method_name, method_params params)
                : return_type(return_type), fn_name(method_name), params(params) {}

        ~function_prototype() = default;

        CUSTOM_CODEGEN(llvm::Function) override;
    };

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
        NODENAME("FUNCTION");
        CHILDREN(prototype, body);

        std::unique_ptr<ast::nodes::function_prototype> prototype;
        scope_block body;

        function(function&&) noexcept = default;
        function(std::unique_ptr<ast::nodes::function_prototype> prototype, scope_block body)
                : prototype(std::move(prototype)), body(std::move(body)) {}

        ~function() = default;
        CODEGEN() override;
    };

    /**
     *  Struct Declaration: Non-Abstract Data Type
     *  ------------------------------------------
     *  A struct defines a new type, containing a list of fields.
     *  This is used to define new types in the program.
     */
    struct struct_declaration : program_level_stmt {
        NODENAME("STRUCT_DECLARATION");
        CHILDREN(fields);

        std::string_view struct_name;
        std::vector<type_instance> fields;

        struct_declaration(struct_declaration&&) noexcept = default;
        struct_declaration(std::string_view name, std::vector<type_instance> fields)
                : struct_name(name), fields(std::move(fields)) {}

        ~struct_declaration() = default;
        CODEGEN() override;
    };

    // -- Root Node -----------------

    struct root : codegen_node {
        NODENAME("ROOT");
        CHILDREN(program_level_statements);

        std::vector<std::unique_ptr<program_level_stmt>> program_level_statements;

        root() noexcept = default;
        root(root&&) noexcept = default;
        ~root() = default;



        CODEGEN() override;
    };
}
