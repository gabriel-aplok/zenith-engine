#pragma once

#include <cstdint>
#include <vector>

#include "math/math.hpp"

namespace Zenith::Render
{
    enum class LightType
    {
        Directional,
        Point,
        Spot,
    };

    struct LightData
    {
        LightType type = LightType::Point;
        Vector3 position{ 0.0f };
        Vector3 direction{ 0.0f, -1.0f, 0.0f };
        Vector4 color{ 1.0f };
        float intensity = 1.0f;
        float range = 10.0f;
        float innerCone = 0.0f;
        float outerCone = 0.0f;
        bool castsShadow = false;
    };

    struct ClusterBuildConfig
    {
        uint16_t clusterX = 16;
        uint16_t clusterY = 9;
        uint16_t clusterZ = 24;
        uint16_t maxLightsPerCluster = 100;
    };

    struct ClusterGridInfo
    {
        Vector2 screenSize{ 0.0f };
        Vector2 clusterSize{ 0.0f };
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        ClusterBuildConfig config{};
    };

    struct ClusteredLightingFrame
    {
        ClusterGridInfo grid{};
        std::vector<LightData> lights{};
    };

} // namespace Zenith::Render
