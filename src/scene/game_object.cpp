#include "scene/game_object.hpp"

#include <algorithm>

#include "scene/scene.hpp"

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

    Component *GameObject::addComponentImpl(std::type_index type, std::unique_ptr<Component> component)
    {
        Component *componentPtr = component.get();
        if (!m_scene)
        {
            return componentPtr;
        }

        component->setOwner(this);
        m_scene->queueComponentAddition(*this, type, std::move(component));
        return componentPtr;
    }

    Component *GameObject::getComponentImpl(std::type_index type)
    {
        return m_scene ? m_scene->getComponent(*this, type) : nullptr;
    }

    const Component *GameObject::getComponentImpl(std::type_index type) const
    {
        return m_scene ? m_scene->getComponent(*this, type) : nullptr;
    }

    bool GameObject::removeComponentImpl(std::type_index type)
    {
        if (!m_scene)
        {
            return false;
        }

        m_scene->queueComponentRemoval(*this, type);
        return true;
    }
} // namespace Zenith
