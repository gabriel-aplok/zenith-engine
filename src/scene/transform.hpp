#pragma once

#include <vector>

#include "math/math.hpp"

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

        const Vector3 &position() const { return m_position; }
        const Quaternion &rotation() const { return m_rotation; }
        const Vector3 &scale() const { return m_scale; }
        Transform *parent() const { return m_parent; }
        const std::vector<Transform *> &children() const { return m_children; }

        void setLocalPosition(const Vector3 &position);
        void setWorldPosition(const Vector3 &position);
        void setLocalRotation(const Quaternion &rotation);
        void setWorldRotation(const Quaternion &rotation);
        void setLocalRotationEulerRadians(const Vector3 &eulerRadians);
        void setWorldRotationEulerRadians(const Vector3 &eulerRadians);
        void setLocalRotationEulerDegrees(const Vector3 &eulerDegrees);
        void setWorldRotationEulerDegrees(const Vector3 &eulerDegrees);
        void setLocalScale(const Vector3 &scale);
        void setWorldScale(const Vector3 &scale);
        void setLocalTransform(const Matrix4 &localTransform);
        void setWorldTransform(const Matrix4 &worldTransform);
        void setParent(Transform *parent, bool keepWorldPosition = true);

        void setPosition(const Vector3 &position) { setLocalPosition(position); }
        void setRotation(const Quaternion &rotation) { setLocalRotation(rotation); }
        void setRotationEulerRadians(const Vector3 &eulerRadians) { setLocalRotationEulerRadians(eulerRadians); }
        void setRotationEulerDegrees(const Vector3 &eulerDegrees) { setLocalRotationEulerDegrees(eulerDegrees); }
        void setScale(const Vector3 &scale) { setLocalScale(scale); }

        void translate(const Vector3 &delta);
        void rotate(const Quaternion &deltaRotation);
        void rotateEulerRadians(const Vector3 &eulerRadians);
        void rotateEulerDegrees(const Vector3 &eulerDegrees);
        void rescale(const Vector3 &deltaScale);
        void lookAt(const Vector3 &target, const Vector3 &worldUp = Vector3{0.0f, 1.0f, 0.0f});

        const Matrix4 &localMatrix() const;
        const Matrix4 &localToWorld() const;
        const Matrix4 &worldMatrix() const;
        Vector3 forward() const;
        Vector3 up() const;
        Vector3 right() const;
        bool isDirty() const { return m_localDirty || m_worldDirty; }

    private:
        void setFromLocalMatrix(const Matrix4 &localTransform);
        void setFromWorldMatrix(const Matrix4 &worldTransform);
        void markLocalDirty();
        void markWorldDirty();
        void updateLocalMatrix() const;
        void updateWorldMatrix() const;

        Vector3 m_position{0.0f};
        Quaternion m_rotation{1.0f, 0.0f, 0.0f, 0.0f};
        Vector3 m_scale{1.0f};
        Transform *m_parent = nullptr;
        std::vector<Transform *> m_children;
        mutable Matrix4 m_localMatrix{1.0f};
        mutable Matrix4 m_localToWorld{1.0f};
        mutable bool m_localDirty = true;
        mutable bool m_worldDirty = true;
    };
} // namespace Zenith
