#include "operator.h"

namespace ast::pm {
    const std::unordered_map<nodes::bin_op_type, uint16_t> binop_prec {
        {nodes::bin_op_type::l_or, 2 },
        {nodes::bin_op_type::l_and, 3 },
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
        { nodes::bin_op_type::shl, 20 },
        { nodes::bin_op_type::shr, 20 },

        { nodes::bin_op_type::pow, 30 }
    };

    const std::unordered_map<std::string_view, nodes::bin_op_type> binop_type_map {
        {"||", nodes::bin_op_type::l_or},
        {"&&", nodes::bin_op_type::l_and},
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
        {"<<", nodes::bin_op_type::shl},
        {">>", nodes::bin_op_type::shr},

        {"**", nodes::bin_op_type::pow},
    };

    const std::unordered_map<std::string_view, nodes::un_op_type> unop_type_map {
        {"*", nodes::un_op_type::dereference},
        {"&", nodes::un_op_type::address_of},
        {"!", nodes::un_op_type::l_not},
        {"-", nodes::un_op_type::negate},
        {"~", nodes::un_op_type::bit_not}
    };

    nodes::un_op_type get_unop(const std::string_view op) {
        const auto find = unop_type_map.find(op);

        if (find == unop_type_map.end())
            return nodes::un_op_type::invalid;

        return find->second;
    }

    nodes::bin_op_type get_binop(const std::string_view op) {
        const auto op_slice = op.ends_with("=") ?
            op.substr(0, op.size() - 1) : op;

        const auto find = binop_type_map.find(op_slice);

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