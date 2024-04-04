#include "abstract_data.h"

#include "data_maps.h"

using namespace ast::nodes;

std::optional<intrinsic_types> ast::nodes::get_intrinsic_type(std::string_view type) {
    const auto it = pm::intrin_map.find(type);

    return it != pm::intrin_map.end() ?
        std::make_optional(it->second) :
        std::nullopt;
}
