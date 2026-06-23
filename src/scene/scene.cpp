#include "scene/scene.hpp"

#include <algorithm>

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

    void Scene::render(RenderFrame &frame, const glm::ivec2 &framebufferSize)
    {
        if (auto *camera = findCamera())
        {
            for (auto &object : m_gameObjects)
            {
                if (object->get_component<Components::Camera>() == camera)
                {
                    frame.clearColor = camera->backgroundColor();
                    camera->buildViewState(object->transform().localToWorld(), framebufferSize, frame.view);
                    frame.commands.setView(frame.view);
                    break;
                }
            }
        }

        for (auto &object : m_gameObjects)
        {
            object->render(frame);
        }
    }
} // namespace Zenith
