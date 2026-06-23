#pragma once

#include "render/render_submission.hpp"

namespace Zenith
{
    class GameObject;

    class Component
    {
    public:
        virtual ~Component();

        virtual void update(float deltaTime) {}
        virtual void render(RenderFrame &frame) {}

        bool enabled() const { return m_enabled; }
        void setEnabled(bool enabled) { m_enabled = enabled; }

        GameObject &gameObject();
        const GameObject &gameObject() const;

    protected:
        Component() = default;

    private:
        friend class GameObject;

        void setOwner(GameObject *owner);

        GameObject *m_owner = nullptr;
        bool m_enabled = true;
    };
} // namespace Zenith
