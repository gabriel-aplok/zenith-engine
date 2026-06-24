#pragma once

namespace Zenith
{
    class GameObject;
    struct RenderFrame;

    class ScriptBehaviour
    {
    public:
        virtual ~ScriptBehaviour() = default;

        virtual void onAdd(GameObject &) {}
        virtual void onRemove(GameObject &) {}
        virtual void onStart(GameObject &) {}
        virtual void onStop(GameObject &) {}
        virtual void onPreUpdate(GameObject &, float) {}
        virtual void onUpdate(GameObject &, float) {}
        virtual void onPostUpdate(GameObject &, float) {}
        virtual void onFixedUpdate(GameObject &, float) {}
        virtual void onRender(GameObject &, RenderFrame &) {}
        virtual void onDispose(GameObject &) {}
    };
} // namespace Zenith
