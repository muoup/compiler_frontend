#include "operator.h"

namespace ast::pm {
    const std::unordered_map<nodes::bin_op_type, uint16_t> binop_prec {
        {nodes::bin_op_type::lor, 2 },
        {nodes::bin_op_type::land, 3 },
        {nodes::bin_op_type::or_, 4 },
        {nodes::bin_op_type::xor_, 5 },
        {nodes::bin_op_type::and_, 6 },

        {nodes::bin_op_type::eq, 7 },
        {nodes::bin_op_type::neq, 7 },

        { nodes::bin_op_type::lt, 8 },
        { nodes::bin_op_type::gt, 8 },
        { nodes::bin_op_type::lte, 8 },
        { nodes::bin_op_type::gte, 8 },

        { nodes::bin_op_type::add, 10 },
        { nodes::bin_op_type::sub, 10 },

        { nodes::bin_op_type::mul, 20 },
        { nodes::bin_op_type::div, 20 },
        { nodes::bin_op_type::mod, 20 },
        { nodes::bin_op_type::sshl, 20 },
        { nodes::bin_op_type::sshr, 20 },

        { nodes::bin_op_type::pow, 30 }
    };

    const std::unordered_map<std::string_view, nodes::bin_op_type> binop_type_map {
        {"||", nodes::bin_op_type::lor},
        {"&&", nodes::bin_op_type::land},
        {"|", nodes::bin_op_type::or_},
        {"^", nodes::bin_op_type::xor_},
        {"&", nodes::bin_op_type::and_},

        {"==", nodes::bin_op_type::eq},
        {"!=", nodes::bin_op_type::neq},
        {"<", nodes::bin_op_type::lt},
        {">", nodes::bin_op_type::gt},
        {"<=", nodes::bin_op_type::lte},
        {">=", nodes::bin_op_type::gte},

        {"+", nodes::bin_op_type::add},
        {"-", nodes::bin_op_type::sub},

        {"*", nodes::bin_op_type::mul},
        {"/", nodes::bin_op_type::div},
        {"%", nodes::bin_op_type::mod},
        {"<<", nodes::bin_op_type::sshl},
        {">>", nodes::bin_op_type::sshr},

        {"**", nodes::bin_op_type::pow},

        {"=", nodes::bin_op_type::assign},
        {"+=", nodes::bin_op_type::assign_add},
        {"-=", nodes::bin_op_type::assign_sub},
        {"*=", nodes::bin_op_type::assign_mul},
        {"/=", nodes::bin_op_type::assign_div},
        {"%=", nodes::bin_op_type::assign_mod},
        {"<<=", nodes::bin_op_type::assign_shift_left},
        {">>=", nodes::bin_op_type::assign_shift_right},
        {"&=", nodes::bin_op_type::assign_and},
        {"|=", nodes::bin_op_type::assign_or},
        {"^=", nodes::bin_op_type::assign_xor}
    };

    const std::unordered_map<std::string_view, nodes::un_op_type> unop_type_map {
        {"*", nodes::un_op_type::dereference},
        {"&", nodes::un_op_type::address_of},
        {"!", nodes::un_op_type::not_},
        {"-", nodes::un_op_type::negate},
        {"~", nodes::un_op_type::bitwise_not}
    };

    nodes::un_op_type get_unop(const std::string_view op) {
        const auto find = unop_type_map.find(op);

        if (find == unop_type_map.end())
            return nodes::un_op_type::invalid;

        return find->second;
    }

    nodes::bin_op_type get_binop(const std::string_view op) {
        const auto find = binop_type_map.find(op);

        if (find == binop_type_map.end())
            return nodes::bin_op_type::invalid;

        return find->second;
    }

    std::string_view from_binop(const nodes::bin_op_type& by_type) {
        for (auto [key, type] : binop_type_map)
            if (type == by_type)
                return key;

        return "OPERATOR NOT FOUND";
    }

    std::string_view from_unop(const nodes::un_op_type& by_type) {
        for (auto [key, type] : unop_type_map)
            if (type == by_type)
                return key;

        return "OPERATOR NOT FOUND";
    }

    int16_t get_prec(const nodes::bin_op& op) {
        const auto find = binop_prec.find(op.type);

        if (find == binop_prec.end())
            return 0;

        return find->second;
    }
}