#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "scene/component.hpp"
#include "scene/transform.hpp"

namespace Zenith
{
    class GameObject
    {
    public:
        explicit GameObject(std::string name = {});
        ~GameObject();

        GameObject(const GameObject &) = delete;
        GameObject &operator=(const GameObject &) = delete;

        GameObject(GameObject &&) noexcept = delete;
        GameObject &operator=(GameObject &&) noexcept = delete;

        const std::string &name() const { return m_name; }
        void setName(std::string name) { m_name = std::move(name); }

        Transform &transform() { return m_transform; }
        const Transform &transform() const { return m_transform; }
        GameObject *parent() const { return m_parent; }
        const std::vector<GameObject *> &children() const { return m_children; }

        bool setParent(GameObject *parent, bool keepWorldPosition = true);
        bool addChild(GameObject &child, bool keepWorldPosition = true);
        bool isDescendantOf(const GameObject &other) const;

        template <typename T, typename... Args>
        T &add_component(Args &&...args)
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T &componentRef = *component;
            componentRef.setOwner(this);
            m_components.emplace_back(std::move(component));
            return componentRef;
        }

        template <typename T>
        T *get_component()
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            for (const auto &component : m_components)
            {
                if (auto *typed = dynamic_cast<T *>(component.get()))
                {
                    return typed;
                }
            }
            return nullptr;
        }

        template <typename T>
        const T *get_component() const
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            for (const auto &component : m_components)
            {
                if (auto *typed = dynamic_cast<const T *>(component.get()))
                {
                    return typed;
                }
            }
            return nullptr;
        }

        template <typename T>
        bool remove_component()
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

            for (auto it = m_components.begin(); it != m_components.end(); ++it)
            {
                if (dynamic_cast<T *>(it->get()) != nullptr)
                {
                    m_components.erase(it);
                    return true;
                }
            }
            return false;
        }

    private:
        std::string m_name;
        Transform m_transform;
        GameObject *m_parent = nullptr;
        std::vector<GameObject *> m_children;
        std::vector<std::unique_ptr<Component>> m_components;
    };
} // namespace Zenith
