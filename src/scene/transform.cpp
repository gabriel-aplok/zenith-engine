#define GLM_ENABLE_EXPERIMENTAL
#include "scene/transform.hpp"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Zenith
{
    namespace
    {
        void decomposeTransform(const Matrix4 &matrix, Vector3 &translation, Quaternion &rotation, Vector3 &scale)
        {
            Vector3 skew{0.0f};
            Vector4 perspective{0.0f};
            Quaternion orientation{1.0f, 0.0f, 0.0f, 0.0f};
            Vector3 tmpScale{1.0f};
            Vector3 tmpTranslation{0.0f};

            if (glm::decompose(matrix, tmpScale, orientation, tmpTranslation, skew, perspective))
            {
                translation = tmpTranslation;
                rotation = glm::normalize(orientation);
                scale = tmpScale;
                return;
            }

            translation = Vector3(matrix[3]);
            rotation = glm::quat_cast(matrix);
            scale = Vector3{1.0f};
        }

        Matrix4 inverseOrIdentity(const Matrix4 &matrix)
        {
            const float determinant = glm::determinant(matrix);
            if (glm::abs(determinant) <= 0.000001f)
            {
                return Matrix4{1.0f};
            }

            return glm::inverse(matrix);
        }
    } // namespace

    Transform::Transform()
    {
        updateLocalMatrix();
        updateWorldMatrix();
    }

    void Transform::setLocalPosition(const Vector3 &position)
    {
        m_position = position;
        markLocalDirty();
    }

    void Transform::setWorldPosition(const Vector3 &position)
    {
        if (m_parent == nullptr)
        {
            setLocalPosition(position);
            return;
        }

        const Matrix4 parentWorldInverse = inverseOrIdentity(m_parent->worldMatrix());
        const Vector4 localPosition = parentWorldInverse * Vector4(position, 1.0f);
        setLocalPosition(Vector3(localPosition));
    }

    void Transform::setLocalRotation(const Quaternion &rotation)
    {
        m_rotation = glm::normalize(rotation);
        markLocalDirty();
    }

    void Transform::setWorldRotation(const Quaternion &rotation)
    {
        if (m_parent == nullptr)
        {
            setLocalRotation(rotation);
            return;
        }

        Vector3 parentPosition{0.0f};
        Quaternion parentRotation{1.0f, 0.0f, 0.0f, 0.0f};
        Vector3 parentScale{1.0f};
        decomposeTransform(m_parent->worldMatrix(), parentPosition, parentRotation, parentScale);
        setLocalRotation(glm::normalize(glm::inverse(parentRotation) * rotation));
    }

    void Transform::setLocalRotationEulerRadians(const Vector3 &eulerRadians)
    {
        setLocalRotation(Quaternion(eulerRadians));
    }

    void Transform::setWorldRotationEulerRadians(const Vector3 &eulerRadians)
    {
        setWorldRotation(Quaternion(eulerRadians));
    }

    void Transform::setLocalRotationEulerDegrees(const Vector3 &eulerDegrees)
    {
        setLocalRotationEulerRadians(glm::radians(eulerDegrees));
    }

    void Transform::setWorldRotationEulerDegrees(const Vector3 &eulerDegrees)
    {
        setWorldRotationEulerRadians(glm::radians(eulerDegrees));
    }

    void Transform::setLocalScale(const Vector3 &scale)
    {
        m_scale = scale;
        markLocalDirty();
    }

    void Transform::setWorldScale(const Vector3 &scale)
    {
        if (m_parent == nullptr)
        {
            setLocalScale(scale);
            return;
        }

        Vector3 parentPosition{0.0f};
        Quaternion parentRotation{1.0f, 0.0f, 0.0f, 0.0f};
        Vector3 parentScale{1.0f};
        decomposeTransform(m_parent->worldMatrix(), parentPosition, parentRotation, parentScale);

        Vector3 localScale = scale;
        if (parentScale.x != 0.0f)
        {
            localScale.x /= parentScale.x;
        }
        if (parentScale.y != 0.0f)
        {
            localScale.y /= parentScale.y;
        }
        if (parentScale.z != 0.0f)
        {
            localScale.z /= parentScale.z;
        }

        setLocalScale(localScale);
    }

    void Transform::setLocalTransform(const Matrix4 &localTransform)
    {
        setFromLocalMatrix(localTransform);
        markLocalDirty();
    }

    void Transform::setWorldTransform(const Matrix4 &worldTransform)
    {
        setFromWorldMatrix(worldTransform);
        markLocalDirty();
    }

    void Transform::setParent(Transform *parent, bool keepWorldPosition)
    {
        if (m_parent == parent || parent == this)
            return;

        for (Transform *ancestor = parent; ancestor != nullptr; ancestor = ancestor->m_parent)
        {
            if (ancestor == this)
                return;
        }

        const Matrix4 worldBeforeReparent = keepWorldPosition ? worldMatrix() : Matrix4{1.0f};

        if (m_parent != nullptr)
        {
            auto &siblings = m_parent->m_children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        }

        m_parent = parent;
        if (m_parent != nullptr)
            m_parent->m_children.push_back(this);

        if (keepWorldPosition)
        {
            setFromWorldMatrix(worldBeforeReparent);
            markLocalDirty();
            return;
        }

        markWorldDirty();
    }

    void Transform::translate(const Vector3 &delta)
    {
        m_position += delta;
        markLocalDirty();
    }

    void Transform::rotate(const Quaternion &deltaRotation)
    {
        m_rotation = glm::normalize(deltaRotation * m_rotation);
        markLocalDirty();
    }

    void Transform::rotateEulerRadians(const Vector3 &eulerRadians)
    {
        rotate(Quaternion(eulerRadians));
    }

    void Transform::rotateEulerDegrees(const Vector3 &eulerDegrees)
    {
        rotateEulerRadians(glm::radians(eulerDegrees));
    }

    void Transform::rescale(const Vector3 &deltaScale)
    {
        m_scale += deltaScale;
        markLocalDirty();
    }

    void Transform::lookAt(const Vector3 &target, const Vector3 &worldUp)
    {
        const Matrix4 view = glm::lookAt(m_position, target, worldUp);
        m_rotation = glm::normalize(glm::quat_cast(glm::inverse(view)));
        markLocalDirty();
    }

    Vector3 Transform::right() const
    {
        return glm::normalize(Vector3{worldMatrix()[0]});
    }

    Vector3 Transform::up() const
    {
        return glm::normalize(Vector3{worldMatrix()[1]});
    }

    Vector3 Transform::forward() const
    {
        return glm::normalize(Vector3{worldMatrix()[2]});
    }

    const Matrix4 &Transform::localMatrix() const
    {
        updateLocalMatrix();
        return m_localMatrix;
    }

    const Matrix4 &Transform::localToWorld() const
    {
        updateWorldMatrix();
        return m_localToWorld;
    }

    const Matrix4 &Transform::worldMatrix() const
    {
        return localToWorld();
    }

    void Transform::setFromLocalMatrix(const Matrix4 &localTransform)
    {
        decomposeTransform(localTransform, m_position, m_rotation, m_scale);
    }

    void Transform::setFromWorldMatrix(const Matrix4 &worldTransform)
    {
        if (m_parent == nullptr)
        {
            setFromLocalMatrix(worldTransform);
            return;
        }

        const Matrix4 localTransform = inverseOrIdentity(m_parent->worldMatrix()) * worldTransform;
        setFromLocalMatrix(localTransform);
    }

    void Transform::markLocalDirty()
    {
        m_localDirty = true;
        markWorldDirty();
    }

    void Transform::markWorldDirty()
    {
        if (!m_worldDirty)
            m_worldDirty = true;

        for (Transform *child : m_children)
            child->markWorldDirty();
    }

    void Transform::updateLocalMatrix() const
    {
        const Matrix4 translation = glm::translate(Matrix4{1.0f}, m_position);
        const Matrix4 rotation = glm::mat4_cast(m_rotation);
        const Matrix4 scaling = glm::scale(Matrix4{1.0f}, m_scale);
        m_localMatrix = translation * rotation * scaling;
        m_localDirty = false;
    }

    void Transform::updateWorldMatrix() const
    {
        if (m_localDirty)
            updateLocalMatrix();

        if (!m_worldDirty)
            return;

        if (m_parent != nullptr)
            m_localToWorld = m_parent->worldMatrix() * m_localMatrix;

        else
            m_localToWorld = m_localMatrix;

        m_worldDirty = false;
    }
} // namespace Zenith
