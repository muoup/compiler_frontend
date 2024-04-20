#include "ast_nodes.h"
#include "../util.h"

using namespace ast::nodes;

value_type method_call::get_type() const {
    return function_types.at(method_name);
}

value_type initialization::get_type() const {
    return variable.type;
}

value_type var_ref::get_type() const {
    return type.value();
}

value_type un_op::get_type() const {
    throw std::runtime_error("Unimplemented!");
}

value_type get_accessor_type(const bin_op &self) {
    auto type = self.left->get_type();

    if (type.is_intrinsic())
        throw std::runtime_error("Cannot access member of intrinsic type!");

    auto struct_name = std::get<std::string_view>(type.type);
    auto &struct_decl = ast::struct_types.at(struct_name);

    auto as_ref = dynamic_cast<const var_ref*>(self.right.get());

    if (!as_ref)
        throw std::runtime_error("Expected variable reference!");

    auto member_name = as_ref->name;

    for (const auto &member : struct_decl) {
        if (member.var_name == member_name)
            return member.type;
    }

    throw std::runtime_error("Member not found!");
}

value_type bin_op::get_type() const {
    if (type == bin_op_type::dot || type == bin_op_type::arrow)
        return get_accessor_type(*this);

    if (left->get_type().is_intrinsic() && right->get_type().is_intrinsic())
        return left->get_type();

    throw std::runtime_error("Unimplemented!");
}

value_type match::get_type() const {
    return value_type { intrinsic_types::void_ };
}

value_type assignment::get_type() const {
    return lhs->get_type();
}

value_type literal::get_type() const {
    switch (value.index()) {
        case UINT:
            return { intrinsic_types::u32 };
        case INT:
            return { intrinsic_types::i32 };
        case FLOAT:
            return { intrinsic_types::f64 };
        case CHAR:
            return { intrinsic_types::char_ };
        case STRING:
            throw std::runtime_error("Unimplemented!");
        default:
            std::unreachable();
    }
}