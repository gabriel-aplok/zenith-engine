#pragma once

#include <cstdint>

#include "engine/debug.hpp"
#include "engine/window.hpp"

namespace Zenith
{
    uint32_t buildBgfxDebugFlags(const EngineDebugConfig &debugConfig);
    void logBgfxStartupDiagnostics(const Window &window, const EngineDebugConfig &debugConfig);

} // namespace Zenith
