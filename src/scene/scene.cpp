#include "scene/scene.hpp"

#include "components/mesh_filter.hpp"

namespace Zenith
{
    GameObject &Scene::createGameObject(std::string name)
    {
        auto object = std::make_unique<GameObject>(std::move(name));
        GameObject &objectRef = *object;
        m_gameObjects.emplace_back(std::move(object));
        return objectRef;
    }

    void Scene::clear()
    {
        for (auto &system : m_systems)
        {
            system->onStop(*this);
            system->onRemove(*this);
        }
        m_gameObjects.clear();
    }

    void Scene::setMeshMetadataProvider(const Render::IMeshMetadataProvider *provider)
    {
        m_meshMetadataProvider = provider;
    }

    void Scene::addSystem(std::unique_ptr<System> system)
    {
        system->onAdd(*this);
        system->onStart(*this);
        m_systems.emplace_back(std::move(system));
    }

    void Scene::setFramebufferSize(const glm::ivec2 &framebufferSize)
    {
        m_framebufferSize = framebufferSize;
    }

    void Scene::update(float deltaTime)
    {
        for (auto &system : m_systems)
        {
            system->preUpdate(*this, deltaTime);
            system->update(*this, deltaTime);
            system->postUpdate(*this, deltaTime);
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

    std::optional<Render::Bounds> Scene::meshBoundsFor(const Components::MeshFilter &filter) const
    {
        if (!m_meshMetadataProvider)
        {
            return std::nullopt;
        }
        return m_meshMetadataProvider->meshBounds(filter.mesh());
    }

    void Scene::render(RenderFrame &frame)
    {
        for (auto &system : m_systems)
        {
            system->preRender(*this, frame);
            system->render(*this, frame);
            system->postRender(*this, frame);
        }
    }
} // namespace Zenith
