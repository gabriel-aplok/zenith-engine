#pragma once

#include <functional>

#include "scene/component.hpp"
#include "render/render_submission.hpp"

namespace Zenith::Components
{
    class ScriptComponent final : public Component
    {
    public:
        using EventFn = std::function<void()>;
        using UpdateFn = std::function<void(float)>;
        using RenderFn = std::function<void(RenderFrame &)>;

        ScriptComponent() = default;
        ~ScriptComponent() override = default;

        EventFn onAdd;
        EventFn onRemove;
        EventFn onStart;
        UpdateFn onUpdate;
        UpdateFn onPreUpdate;
        UpdateFn onPostUpdate;
        UpdateFn onFixedUpdate;
        RenderFn onRender;
        EventFn onDispose;

        bool started = false;

    private:
    };
} // namespace Zenith::Components
