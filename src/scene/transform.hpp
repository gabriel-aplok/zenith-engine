#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Zenith
{
    class Transform
    {
    public:
        Transform();
        ~Transform() = default;

        Transform(const Transform &) = delete;
        Transform &operator=(const Transform &) = delete;

        Transform(Transform &&) noexcept = delete;
        Transform &operator=(Transform &&) noexcept = delete;

        const glm::vec3 &position() const { return m_position; }
        const glm::quat &rotation() const { return m_rotation; }
        const glm::vec3 &scale() const { return m_scale; }
        Transform *parent() const { return m_parent; }
        const std::vector<Transform *> &children() const { return m_children; }

        void setPosition(const glm::vec3 &position);
        void setRotation(const glm::quat &rotation);
        void setRotationEulerRadians(const glm::vec3 &eulerRadians);
        void setRotationEulerDegrees(const glm::vec3 &eulerDegrees);
        void setScale(const glm::vec3 &scale);
        void setParent(Transform *parent);

        void translate(const glm::vec3 &delta);
        void rotate(const glm::quat &deltaRotation);
        void rotateEulerRadians(const glm::vec3 &eulerRadians);
        void rotateEulerDegrees(const glm::vec3 &eulerDegrees);
        void rescale(const glm::vec3 &deltaScale);
        void lookAt(const glm::vec3 &target, const glm::vec3 &worldUp = glm::vec3{0.0f, 1.0f, 0.0f});

        const glm::mat4 &localMatrix() const;
        const glm::mat4 &localToWorld() const;
        const glm::mat4 &worldMatrix() const;
        glm::vec3 forward() const;
        glm::vec3 up() const;
        glm::vec3 right() const;
        bool isDirty() const { return m_localDirty || m_worldDirty; }

    private:
        void markLocalDirty();
        void markWorldDirty();
        void updateLocalMatrix() const;
        void updateWorldMatrix() const;

        glm::vec3 m_position{0.0f};
        glm::quat m_rotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 m_scale{1.0f};
        Transform *m_parent = nullptr;
        std::vector<Transform *> m_children;
        mutable glm::mat4 m_localMatrix{1.0f};
        mutable glm::mat4 m_localToWorld{1.0f};
        mutable bool m_localDirty = true;
        mutable bool m_worldDirty = true;
    };
} // namespace Zenith
