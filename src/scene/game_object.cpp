#include "scene/game_object.hpp"

namespace Zenith
{
    GameObject::GameObject(std::string name)
        : m_name(std::move(name))
    {
    }

    GameObject::~GameObject() = default;

    void GameObject::update(float deltaTime)
    {
        for (auto &component : m_components)
        {
            component->update(deltaTime);
        }
    }

    void GameObject::render(RenderFrame &frame)
    {
        for (auto &component : m_components)
        {
            component->render(frame);
        }
    }
} // namespace Zenith
