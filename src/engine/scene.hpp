#pragma once
#include "engine/entity.hpp"
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <stdexcept>
#include <any>

namespace Zenith{

    class Scene {
    public:
        Entity createEntity();
        void destroyEntity(Entity entity);

        template <typename T>
        void addComponent(Entity entity, T &&component);

        template <typename T>
        T &getComponent(Entity entity);

        template <typename T>
        bool hasComponent(Entity entity) const;

        // Call each frame
        void update(float dt);

    private:
        uint32_t m_nextEntity = 1;
        std::unordered_map<Entity, std::unordered_map<std::type_index, std::any>> m_components;
    };

} // namespace Zenith