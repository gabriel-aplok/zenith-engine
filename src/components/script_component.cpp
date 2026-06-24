#include "components/script_component.hpp"

#include "components/script_behaviour.hpp"
#include "scene/component.hpp"

namespace Zenith::Components
{
    ScriptComponent::~ScriptComponent()
    {
        clearBehaviour();
    }

    void ScriptComponent::clearBehaviour()
    {
        if (auto *behaviour = m_behaviour.get())
        {
            if (auto *objectOwner = owner())
            {
                if (started)
                {
                    behaviour->onStop(*objectOwner);
                }
            }
        }

        m_behaviour.reset();
        started = false;
    }
} // namespace Zenith::Components
