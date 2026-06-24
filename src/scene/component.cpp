#include "scene/component.hpp"

#include "scene/game_object.hpp"
#include "scene/scene.hpp"

namespace Zenith
{
    Component::~Component() = default;

    GameObject &Component::gameObject()
    {
        return *m_owner;
    }

    const GameObject &Component::gameObject() const
    {
        return *m_owner;
    }

    Scene *Component::scene()
    {
        return m_owner ? m_owner->scene() : nullptr;
    }

    const Scene *Component::scene() const
    {
        return m_owner ? m_owner->scene() : nullptr;
    }

    void Component::setOwner(GameObject *owner)
    {
        m_owner = owner;
    }
} // namespace Zenith
