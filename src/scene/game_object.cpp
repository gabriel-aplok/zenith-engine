#include "scene/game_object.hpp"

#include <algorithm>

#include "components/script_component.hpp"
#include "components/script_behaviour.hpp"

namespace Zenith
{
    GameObject::GameObject(std::string name)
        : m_name(std::move(name))
    {
    }

    GameObject::~GameObject() = default;

    bool GameObject::setParent(GameObject *parent, bool keepWorldPosition)
    {
        if (parent == this)
        {
            return false;
        }

        if (parent != nullptr && parent->isDescendantOf(*this))
        {
            return false;
        }

        if (m_parent == parent)
        {
            return true;
        }

        if (m_parent != nullptr)
        {
            auto &siblings = m_parent->m_children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        }

        m_parent = parent;
        if (m_parent != nullptr)
        {
            m_parent->m_children.push_back(this);
        }

        m_transform.setParent(m_parent != nullptr ? &m_parent->transform() : nullptr, keepWorldPosition);
        return true;
    }

    bool GameObject::addChild(GameObject &child, bool keepWorldPosition)
    {
        return child.setParent(this, keepWorldPosition);
    }

    bool GameObject::isDescendantOf(const GameObject &other) const
    {
        for (const GameObject *current = m_parent; current != nullptr; current = current->m_parent)
        {
            if (current == &other)
            {
                return true;
            }
        }

        return false;
    }

    void GameObject::notifyComponentAdded(Component &component)
    {
        if (auto *script = dynamic_cast<Components::ScriptComponent *>(&component))
        {
            if (auto *behaviour = script->behaviour())
            {
                behaviour->onAdd(*this);
            }
        }
    }

    void GameObject::notifyComponentRemoved(Component &component)
    {
        if (auto *script = dynamic_cast<Components::ScriptComponent *>(&component))
        {
            if (auto *behaviour = script->behaviour())
            {
                behaviour->onRemove(*this);
                behaviour->onDispose(*this);
            }
        }
    }

} // namespace Zenith
