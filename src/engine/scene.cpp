#include "engine/scene.hpp"

namespace Zenith
{

    Entity Scene::createEntity()
    {
        return m_nextEntity++;
    }

    void Scene::destroyEntity(Entity entity)
    {
        m_components.erase(entity);
    }

    template <typename T>
    void Scene::addComponent(Entity entity, T &&component)
    {
        m_components[entity][std::type_index(typeid(T))] = std::forward<T>(component);
    }

    template <typename T>
    T &Scene::getComponent(Entity entity)
    {
        auto &compMap = m_components.at(entity);
        return std::any_cast<T &>(compMap.at(std::type_index(typeid(T))));
    }

    template <typename T>
    bool Scene::hasComponent(Entity entity) const
    {
        auto it = m_components.find(entity);
        if (it == m_components.end())
            return false;
        return it->second.contains(std::type_index(typeid(T)));
    }

    void Scene::update(float dt)
    {
        // Example: update all transforms, physics, etc.
    }

} // namespace Zenith