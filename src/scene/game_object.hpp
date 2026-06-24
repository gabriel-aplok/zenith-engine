#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <type_traits>
#include <utility>
#include <vector>

#include "scene/component.hpp"
#include "scene/transform.hpp"

namespace Zenith
{
    class GameObject;
    class Scene;

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
        Scene *scene() const { return m_scene; }

        bool setParent(GameObject *parent, bool keepWorldPosition = true);
        bool addChild(GameObject &child, bool keepWorldPosition = true);
        bool isDescendantOf(const GameObject &other) const;

        template <typename T, typename... Args>
        T &add_component(Args &&...args)
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T &componentRef = *component;
            addComponentImpl(std::type_index(typeid(T)), std::move(component));
            return componentRef;
        }

        template <typename T>
        T *get_component()
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            return dynamic_cast<T *>(getComponentImpl(std::type_index(typeid(T))));
        }

        template <typename T>
        const T *get_component() const
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            return dynamic_cast<const T *>(getComponentImpl(std::type_index(typeid(T))));
        }

        template <typename T>
        bool remove_component()
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            return removeComponentImpl(std::type_index(typeid(T)));
        }

    private:
        Component *addComponentImpl(std::type_index type, std::unique_ptr<Component> component);
        Component *getComponentImpl(std::type_index type);
        const Component *getComponentImpl(std::type_index type) const;
        bool removeComponentImpl(std::type_index type);

        std::string m_name;
        Transform m_transform;
        Scene *m_scene = nullptr;
        GameObject *m_parent = nullptr;
        std::vector<GameObject *> m_children;

        friend class Scene;
    };
} // namespace Zenith
