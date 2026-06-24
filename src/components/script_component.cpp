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

    void ScriptComponent::markAttached()
    {
        m_attached = true;
        if (m_behaviour && owner())
        {
            m_behaviour->onAdd(*owner());
        }
    }

    void ScriptComponent::markDetached()
    {
        if (m_behaviour && owner())
        {
            if (started)
            {
                m_behaviour->onStop(*owner());
            }
            m_behaviour->onRemove(*owner());
        }
        m_attached = false;
        started = false;
    }
} // namespace Zenith::Components
