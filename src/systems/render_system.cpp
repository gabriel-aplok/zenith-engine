#include "systems/render_system.hpp"

#include <algorithm>
#include <array>

#include "components/mesh_filter.hpp"
#include "components/mesh_renderer.hpp"
#include "scene/scene.hpp"

namespace Zenith
{
    namespace
    {
        struct FrustumPlane
        {
            Vector3 normal{0.0f};
            float distance = 0.0f;
        };

        std::array<FrustumPlane, 6> buildFrustumPlanes(const Matrix4 &viewProjection)
        {
            const Vector4 row0 = Vector4(viewProjection[0][0], viewProjection[1][0], viewProjection[2][0], viewProjection[3][0]);
            const Vector4 row1 = Vector4(viewProjection[0][1], viewProjection[1][1], viewProjection[2][1], viewProjection[3][1]);
            const Vector4 row2 = Vector4(viewProjection[0][2], viewProjection[1][2], viewProjection[2][2], viewProjection[3][2]);
            const Vector4 row3 = Vector4(viewProjection[0][3], viewProjection[1][3], viewProjection[2][3], viewProjection[3][3]);

            const Vector4 rawPlanes[6] = {row3 + row0, row3 - row0, row3 + row1, row3 - row1, row3 + row2, row3 - row2};
            std::array<FrustumPlane, 6> planes{};
            for (std::size_t i = 0; i < planes.size(); ++i)
            {
                const Vector3 normal = Vector3(rawPlanes[i]);
                const float length = glm::length(normal);
                if (length > 0.0f)
                {
                    planes[i].normal = normal / length;
                    planes[i].distance = rawPlanes[i].w / length;
                }
            }
            return planes;
        }

        bool sphereInFrustum(const std::array<FrustumPlane, 6> &planes, const Vector3 &center, float radius)
        {
            for (const FrustumPlane &plane : planes)
            {
                if (glm::dot(plane.normal, center) + plane.distance < -radius)
                {
                    return false;
                }
            }
            return true;
        }

        float maxWorldScale(const Matrix4 &worldMatrix)
        {
            const float sx = glm::length(Vector3(worldMatrix[0]));
            const float sy = glm::length(Vector3(worldMatrix[1]));
            const float sz = glm::length(Vector3(worldMatrix[2]));
            return glm::max(sx, glm::max(sy, sz));
        }
    } // namespace

    void RenderSystem::render(Scene &scene, RenderFrame &frame)
    {
        std::array<FrustumPlane, 6> frustum = buildFrustumPlanes(frame.view.projection * frame.view.view);

        for (auto &objectPtr : scene.gameObjects())
        {
            auto &object = *objectPtr;
            auto *filter = object.get_component<Components::MeshFilter>();
            auto *renderer = object.get_component<Components::MeshRenderer>();
            if (filter == nullptr || renderer == nullptr)
            {
                continue;
            }

            const auto bounds = filter->bounds();
            if (bounds.has_value())
            {
                const Matrix4 &world = object.transform().localToWorld();
                const Render::Bounds &meshBounds = *bounds;
                const Vector3 center = Vector3(world * Vector4(meshBounds.center, 1.0f));
                const float radius = meshBounds.radius() * maxWorldScale(world);
                if (!sphereInFrustum(frustum, center, radius))
                {
                    continue;
                }
            }

            frame.submitMesh(filter->mesh(), object.transform().localToWorld(), renderer->material());
        }
    }
} // namespace Zenith
