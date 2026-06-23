#pragma once

#include <string>

#include "resource/image_source.hpp"

namespace Zenith
{
    struct TextureAsset
    {
        ImageSource source;
        std::string sourcePath;
    };
} // namespace Zenith
