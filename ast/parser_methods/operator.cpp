#include "operator.h"

namespace ast::pm {
    const std::unordered_map<nodes::operator_type, uint16_t> binop_prec {
        {nodes::operator_type::lor, 2 },
        {nodes::operator_type::land, 3 },
        {nodes::operator_type::or_, 4 },
        {nodes::operator_type::xor_, 5 },
        {nodes::operator_type::and_, 6 },

        {nodes::operator_type::eq, 7 },
        {nodes::operator_type::neq, 7 },

        { nodes::operator_type::lt, 8 },
        { nodes::operator_type::gt, 8 },
        { nodes::operator_type::lte, 8 },
        { nodes::operator_type::gte, 8 },

        { nodes::operator_type::add, 10 },
        { nodes::operator_type::sub, 10 },

        { nodes::operator_type::mul, 20 },
        { nodes::operator_type::div, 20 },
        { nodes::operator_type::mod, 20 },
        { nodes::operator_type::sshl, 20 },
        { nodes::operator_type::sshr, 20 },

        { nodes::operator_type::pow, 30 }
    };

    const std::unordered_map<std::string_view, nodes::operator_type> binop_type_map {
        {"||", nodes::operator_type::lor},
        {"&&", nodes::operator_type::land},
        {"|", nodes::operator_type::or_},
        {"^", nodes::operator_type::xor_},
        {"&", nodes::operator_type::and_},

        {"==", nodes::operator_type::eq},
        {"!=", nodes::operator_type::neq},
        {"<", nodes::operator_type::lt},
        {">", nodes::operator_type::gt},
        {"<=", nodes::operator_type::lte},
        {">=", nodes::operator_type::gte},

        {"+", nodes::operator_type::add},
        {"-", nodes::operator_type::sub},

        {"*", nodes::operator_type::mul},
        {"/", nodes::operator_type::div},
        {"%", nodes::operator_type::mod},
        {"<<", nodes::operator_type::sshl},
        {">>", nodes::operator_type::sshr},

        {"**", nodes::operator_type::pow},

        {"=", nodes::operator_type::assign},
        {"+=", nodes::operator_type::assign_add},
        {"-=", nodes::operator_type::assign_sub},
        {"*=", nodes::operator_type::assign_mul},
        {"/=", nodes::operator_type::assign_div},
        {"%=", nodes::operator_type::assign_mod},
        {"<<=", nodes::operator_type::assign_shift_left},
        {">>=", nodes::operator_type::assign_shift_right},
        {"&=", nodes::operator_type::assign_and},
        {"|=", nodes::operator_type::assign_or},
        {"^=", nodes::operator_type::assign_xor}
    };

    nodes::operator_type get_op(const std::string_view op) {
        const auto find = binop_type_map.find(op);

        if (find == binop_type_map.end())
            return nodes::operator_type::invalid;

        return find->second;
    }

    int16_t get_prec(const nodes::op op) {
        const auto find = binop_prec.find(op.type);

        if (find == binop_prec.end())
            return 0;

        return find->second;
    }
}