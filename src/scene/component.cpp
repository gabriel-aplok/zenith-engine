#include "scene/component.hpp"

#include "scene/game_object.hpp"

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

    void Component::setOwner(GameObject *owner)
    {
        m_owner = owner;
    }
} // namespace Zenith
