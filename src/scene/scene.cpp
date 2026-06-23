#include "scene/scene.hpp"

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

    void Scene::render(RenderFrame &frame)
    {
        for (auto &object : m_gameObjects)
        {
            object->render(frame);
        }
    }
} // namespace Zenith
