#pragma once

#include <memory>
#include <type_traits>
#include <utility>

#include "components/script_behaviour.hpp"
#include "scene/component.hpp"

namespace Zenith
{
    namespace Components
    {
        class ScriptComponent final : public Component
        {
        public:
            ScriptComponent() = default;
            ~ScriptComponent() override;

            ScriptBehaviour *behaviour() { return m_behaviour.get(); }
            const ScriptBehaviour *behaviour() const { return m_behaviour.get(); }

            template <typename T, typename... Args>
            T &setBehaviour(Args &&...args)
            {
                static_assert(std::is_base_of_v<ScriptBehaviour, T>, "T must derive from ScriptBehaviour");
                clearBehaviour();
                auto behaviour = std::make_unique<T>(std::forward<Args>(args)...);
                T &behaviourRef = *behaviour;
                m_behaviour = std::move(behaviour);
                return behaviourRef;
            }

            template <typename T>
            T *getBehaviour()
            {
                static_assert(std::is_base_of_v<ScriptBehaviour, T>, "T must derive from ScriptBehaviour");
                return dynamic_cast<T *>(m_behaviour.get());
            }

            template <typename T>
            const T *getBehaviour() const
            {
                static_assert(std::is_base_of_v<ScriptBehaviour, T>, "T must derive from ScriptBehaviour");
                return dynamic_cast<const T *>(m_behaviour.get());
            }

            void clearBehaviour();

            bool started = false;

        private:
            std::unique_ptr<ScriptBehaviour> m_behaviour;
        };
    } // namespace Components
} // namespace Zenith
