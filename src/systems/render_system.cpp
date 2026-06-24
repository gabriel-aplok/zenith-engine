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
            glm::vec3 normal{0.0f};
            float distance = 0.0f;
        };

        std::array<FrustumPlane, 6> buildFrustumPlanes(const glm::mat4 &viewProjection)
        {
            const glm::vec4 row0 = glm::vec4(viewProjection[0][0], viewProjection[1][0], viewProjection[2][0], viewProjection[3][0]);
            const glm::vec4 row1 = glm::vec4(viewProjection[0][1], viewProjection[1][1], viewProjection[2][1], viewProjection[3][1]);
            const glm::vec4 row2 = glm::vec4(viewProjection[0][2], viewProjection[1][2], viewProjection[2][2], viewProjection[3][2]);
            const glm::vec4 row3 = glm::vec4(viewProjection[0][3], viewProjection[1][3], viewProjection[2][3], viewProjection[3][3]);

            const glm::vec4 rawPlanes[6] = {row3 + row0, row3 - row0, row3 + row1, row3 - row1, row3 + row2, row3 - row2};
            std::array<FrustumPlane, 6> planes{};
            for (std::size_t i = 0; i < planes.size(); ++i)
            {
                const glm::vec3 normal = glm::vec3(rawPlanes[i]);
                const float length = glm::length(normal);
                if (length > 0.0f)
                {
                    planes[i].normal = normal / length;
                    planes[i].distance = rawPlanes[i].w / length;
                }
            }
            return planes;
        }

        bool sphereInFrustum(const std::array<FrustumPlane, 6> &planes, const glm::vec3 &center, float radius)
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

        float maxWorldScale(const glm::mat4 &worldMatrix)
        {
            const float sx = glm::length(glm::vec3(worldMatrix[0]));
            const float sy = glm::length(glm::vec3(worldMatrix[1]));
            const float sz = glm::length(glm::vec3(worldMatrix[2]));
            return glm::max(sx, glm::max(sy, sz));
        }
    } // namespace

    void RenderSystem::render(Scene &scene, RenderFrame &frame)
    {
        std::array<FrustumPlane, 6> frustum = buildFrustumPlanes(frame.view.projection * frame.view.view);

        for (auto &objectPtr : scene.gameObjects())
        {
            auto &object = *objectPtr;
            auto *filter = object.scene() ? object.scene()->tryGetComponent<Components::MeshFilter>(object) : object.get_component<Components::MeshFilter>();
            auto *renderer = object.scene() ? object.scene()->tryGetComponent<Components::MeshRenderer>(object) : object.get_component<Components::MeshRenderer>();
            if (filter == nullptr || renderer == nullptr)
            {
                continue;
            }

            const auto bounds = filter->bounds();
            if (bounds.has_value())
            {
                const glm::mat4 &world = object.transform().localToWorld();
                const Render::Bounds &meshBounds = *bounds;
                const glm::vec3 center = glm::vec3(world * glm::vec4(meshBounds.center, 1.0f));
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
