#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "resource/resource_source.hpp"

namespace Zenith
{
    struct ImageSourceData
    {
        std::uint32_t width = 0;
        std::uint32_t height = 0;
        std::vector<std::uint8_t> pixels;
        std::string format;
    };

    using ImageSource = ResourceSource<ImageSourceData>;
} // namespace Zenith
