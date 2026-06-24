#pragma once

namespace Zenith
{
    class GameObject;

    class Component
    {
    public:
        virtual ~Component();

        bool enabled() const { return m_enabled; }
        void setEnabled(bool enabled) { m_enabled = enabled; }

        GameObject &gameObject();
        const GameObject &gameObject() const;

    protected:
        Component() = default;
        GameObject *owner() const { return m_owner; }

    private:
        friend class Scene;
        friend class GameObject;

        void setOwner(GameObject *owner);

        GameObject *m_owner = nullptr;
        bool m_enabled = true;
    };
} // namespace Zenith
