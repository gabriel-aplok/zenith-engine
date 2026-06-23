#pragma once

#include <cstdint>
#include <vector>

#include "resource/resource_source.hpp"

namespace Zenith
{
    using BinarySource = ResourceSource<std::vector<std::uint8_t>>;
} // namespace Zenith
