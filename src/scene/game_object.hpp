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
    void queueComponentAddition(Scene &scene, GameObject &object, std::type_index type, std::unique_ptr<Component> component);
    void queueComponentRemoval(Scene &scene, GameObject &object, std::type_index type);
    Component *sceneGetComponent(Scene &scene, GameObject &object, std::type_index type);
    const Component *sceneGetComponent(const Scene &scene, const GameObject &object, std::type_index type);

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
            componentRef.setOwner(this);
            if (m_scene)
            {
                queueComponentAddition(*m_scene, *this, std::type_index(typeid(T)), std::move(component));
            }
            return componentRef;
        }

        template <typename T>
        T *get_component()
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            if (m_scene)
            {
                if (auto *component = sceneGetComponent(*m_scene, *this, std::type_index(typeid(T))))
                {
                    return dynamic_cast<T *>(component);
                }
            }
            return nullptr;
        }

        template <typename T>
        const T *get_component() const
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            if (m_scene)
            {
                if (auto *component = sceneGetComponent(*m_scene, *this, std::type_index(typeid(T))))
                {
                    return dynamic_cast<const T *>(component);
                }
            }
            return nullptr;
        }

        template <typename T>
        bool remove_component()
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            if (m_scene)
            {
                queueComponentRemoval(*m_scene, *this, std::type_index(typeid(T)));
                return true;
            }
            return false;
        }

    private:
        std::string m_name;
        Transform m_transform;
        Scene *m_scene = nullptr;
        GameObject *m_parent = nullptr;
        std::vector<GameObject *> m_children;

        friend class Scene;
    };
} // namespace Zenith
