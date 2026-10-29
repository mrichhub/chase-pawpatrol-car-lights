#pragma once

#include <cstddef>

namespace pixel_mapping {

// Mappings belong to effects, not to the engine or to physical group objects.
template <std::size_t N>
bool contains(const std::size_t (&indices)[N], std::size_t index) {
    for (const auto candidate : indices) {
        if (candidate == index) {
            return true;
        }
    }
    return false;
}

}  // namespace pixel_mapping
