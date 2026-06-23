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

        GameObject &gameObject();
        const GameObject &gameObject() const;

    protected:
        Component() = default;

    private:
        friend class GameObject;

        void setOwner(GameObject *owner);

        GameObject *m_owner = nullptr;
    };
} // namespace Zenith
