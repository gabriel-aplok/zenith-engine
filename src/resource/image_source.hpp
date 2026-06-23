#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "resource/resource_source.hpp"

namespace Zenith
{
    struct ImageSourceData
    {
        std::vector<std::uint8_t> bytes;
        std::string format;
    };

    using ImageSource = ResourceSource<ImageSourceData>;
} // namespace Zenith
