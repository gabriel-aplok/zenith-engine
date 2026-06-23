#include "scene/scene.hpp"

#include <array>
#include <algorithm>

#include "components/mesh_filter.hpp"
#include "components/mesh_renderer.hpp"

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

            const glm::vec4 rawPlanes[6] = {
                row3 + row0,
                row3 - row0,
                row3 + row1,
                row3 - row1,
                row3 + row2,
                row3 - row2,
            };

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

        Render::RenderClearState buildClearState(const Components::Camera &camera)
        {
            Render::RenderClearState clear{};
            clear.color = camera.backgroundColor();

            switch (camera.clearFlags())
            {
            case Components::Camera::ClearFlags::Skybox:
            case Components::Camera::ClearFlags::SolidColor:
                clear.flags = 0x01 | 0x02;
                break;
            case Components::Camera::ClearFlags::DepthOnly:
                clear.flags = 0x02;
                break;
            case Components::Camera::ClearFlags::Nothing:
            default:
                clear.flags = 0x00;
                break;
            }

            return clear;
        }
    } // namespace

    GameObject &Scene::createGameObject(std::string name)
    {
        auto object = std::make_unique<GameObject>(std::move(name));
        GameObject &objectRef = *object;
        m_gameObjects.emplace_back(std::move(object));
        return objectRef;
    }

    void Scene::clear()
    {
        m_gameObjects.clear();
    }

    void Scene::update(float deltaTime)
    {
        for (auto &object : m_gameObjects)
        {
            object->update(deltaTime);
        }
    }

    Components::Camera *Scene::findCamera()
    {
        Components::Camera *bestCamera = nullptr;
        float bestDepth = 0.0f;

        for (auto &object : m_gameObjects)
        {
            if (auto *camera = object->get_component<Components::Camera>())
            {
                if (!camera->enabled())
                {
                    continue;
                }

                if (!bestCamera || camera->depth() < bestDepth)
                {
                    bestCamera = camera;
                    bestDepth = camera->depth();
                }
            }
        }

        return bestCamera;
    }

    const Components::Camera *Scene::findCamera() const
    {
        return const_cast<Scene *>(this)->findCamera();
    }

    bool Scene::buildRenderFrame(RenderFrame &frame, const glm::ivec2 &framebufferSize)
    {
        if (auto *camera = findCamera())
        {
            for (auto &object : m_gameObjects)
            {
                if (object->get_component<Components::Camera>() == camera)
                {
                    frame.clear = buildClearState(*camera);
                    const glm::mat4 cameraWorld = object->transform().localToWorld();
                    camera->buildViewState(cameraWorld, framebufferSize, frame.view);
                    frame.commands.setView(frame.view);
                    return true;
                }
            }
        }

        return false;
    }

    void Scene::render(RenderFrame &frame)
    {
        std::array<FrustumPlane, 6> frustum = buildFrustumPlanes(frame.view.projection * frame.view.view);

        for (auto &object : m_gameObjects)
        {
            auto *filter = object->get_component<Components::MeshFilter>();
            auto *renderer = object->get_component<Components::MeshRenderer>();
            if (filter != nullptr && renderer != nullptr)
            {
                const glm::mat4 &world = object->transform().localToWorld();
                const glm::vec3 center = glm::vec3(world * glm::vec4(filter->bounds().center, 1.0f));
                const float radius = filter->bounds().radius() * maxWorldScale(world);
                if (!sphereInFrustum(frustum, center, radius))
                {
                    continue;
                }
            }

            object->render(frame);
        }
    }

    void Scene::render(RenderFrame &frame, const glm::ivec2 &framebufferSize)
    {
        if (!buildRenderFrame(frame, framebufferSize))
        {
            return;
        }

        render(frame);
    }
} // namespace Zenith
