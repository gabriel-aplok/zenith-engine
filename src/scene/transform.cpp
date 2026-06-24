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
        void decomposeTransform(const glm::mat4 &matrix, glm::vec3 &translation, glm::quat &rotation, glm::vec3 &scale)
        {
            glm::vec3 skew{0.0f};
            glm::vec4 perspective{0.0f};
            glm::quat orientation{1.0f, 0.0f, 0.0f, 0.0f};
            glm::vec3 tmpScale{1.0f};
            glm::vec3 tmpTranslation{0.0f};

            if (glm::decompose(matrix, tmpScale, orientation, tmpTranslation, skew, perspective))
            {
                translation = tmpTranslation;
                rotation = glm::normalize(orientation);
                scale = tmpScale;
                return;
            }

            translation = glm::vec3(matrix[3]);
            rotation = glm::quat_cast(matrix);
            scale = glm::vec3{1.0f};
        }

        glm::mat4 inverseOrIdentity(const glm::mat4 &matrix)
        {
            const float determinant = glm::determinant(matrix);
            if (glm::abs(determinant) <= 0.000001f)
            {
                return glm::mat4{1.0f};
            }

            return glm::inverse(matrix);
        }
    } // namespace

    Transform::Transform()
    {
        updateLocalMatrix();
        updateWorldMatrix();
    }

    void Transform::setLocalPosition(const glm::vec3 &position)
    {
        m_position = position;
        markLocalDirty();
    }

    void Transform::setWorldPosition(const glm::vec3 &position)
    {
        if (m_parent == nullptr)
        {
            setLocalPosition(position);
            return;
        }

        const glm::mat4 parentWorldInverse = inverseOrIdentity(m_parent->worldMatrix());
        const glm::vec4 localPosition = parentWorldInverse * glm::vec4(position, 1.0f);
        setLocalPosition(glm::vec3(localPosition));
    }

    void Transform::setLocalRotation(const glm::quat &rotation)
    {
        m_rotation = glm::normalize(rotation);
        markLocalDirty();
    }

    void Transform::setWorldRotation(const glm::quat &rotation)
    {
        if (m_parent == nullptr)
        {
            setLocalRotation(rotation);
            return;
        }

        glm::vec3 parentPosition{0.0f};
        glm::quat parentRotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 parentScale{1.0f};
        decomposeTransform(m_parent->worldMatrix(), parentPosition, parentRotation, parentScale);
        setLocalRotation(glm::normalize(glm::inverse(parentRotation) * rotation));
    }

    void Transform::setLocalRotationEulerRadians(const glm::vec3 &eulerRadians)
    {
        setLocalRotation(glm::quat(eulerRadians));
    }

    void Transform::setWorldRotationEulerRadians(const glm::vec3 &eulerRadians)
    {
        setWorldRotation(glm::quat(eulerRadians));
    }

    void Transform::setLocalRotationEulerDegrees(const glm::vec3 &eulerDegrees)
    {
        setLocalRotationEulerRadians(glm::radians(eulerDegrees));
    }

    void Transform::setWorldRotationEulerDegrees(const glm::vec3 &eulerDegrees)
    {
        setWorldRotationEulerRadians(glm::radians(eulerDegrees));
    }

    void Transform::setLocalScale(const glm::vec3 &scale)
    {
        m_scale = scale;
        markLocalDirty();
    }

    void Transform::setWorldScale(const glm::vec3 &scale)
    {
        if (m_parent == nullptr)
        {
            setLocalScale(scale);
            return;
        }

        glm::vec3 parentPosition{0.0f};
        glm::quat parentRotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 parentScale{1.0f};
        decomposeTransform(m_parent->worldMatrix(), parentPosition, parentRotation, parentScale);

        glm::vec3 localScale = scale;
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

    void Transform::setLocalTransform(const glm::mat4 &localTransform)
    {
        setFromLocalMatrix(localTransform);
        markLocalDirty();
    }

    void Transform::setWorldTransform(const glm::mat4 &worldTransform)
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

        const glm::mat4 worldBeforeReparent = keepWorldPosition ? worldMatrix() : glm::mat4{1.0f};

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

    void Transform::translate(const glm::vec3 &delta)
    {
        m_position += delta;
        markLocalDirty();
    }

    void Transform::rotate(const glm::quat &deltaRotation)
    {
        m_rotation = glm::normalize(deltaRotation * m_rotation);
        markLocalDirty();
    }

    void Transform::rotateEulerRadians(const glm::vec3 &eulerRadians)
    {
        rotate(glm::quat(eulerRadians));
    }

    void Transform::rotateEulerDegrees(const glm::vec3 &eulerDegrees)
    {
        rotateEulerRadians(glm::radians(eulerDegrees));
    }

    void Transform::rescale(const glm::vec3 &deltaScale)
    {
        m_scale += deltaScale;
        markLocalDirty();
    }

    void Transform::lookAt(const glm::vec3 &target, const glm::vec3 &worldUp)
    {
        const glm::mat4 view = glm::lookAt(m_position, target, worldUp);
        m_rotation = glm::normalize(glm::quat_cast(glm::inverse(view)));
        markLocalDirty();
    }

    glm::vec3 Transform::right() const
    {
        return glm::normalize(glm::vec3{worldMatrix()[0]});
    }

    glm::vec3 Transform::up() const
    {
        return glm::normalize(glm::vec3{worldMatrix()[1]});
    }

    glm::vec3 Transform::forward() const
    {
        return glm::normalize(glm::vec3{worldMatrix()[2]});
    }

    const glm::mat4 &Transform::localMatrix() const
    {
        updateLocalMatrix();
        return m_localMatrix;
    }

    const glm::mat4 &Transform::localToWorld() const
    {
        updateWorldMatrix();
        return m_localToWorld;
    }

    const glm::mat4 &Transform::worldMatrix() const
    {
        return localToWorld();
    }

    void Transform::setFromLocalMatrix(const glm::mat4 &localTransform)
    {
        decomposeTransform(localTransform, m_position, m_rotation, m_scale);
    }

    void Transform::setFromWorldMatrix(const glm::mat4 &worldTransform)
    {
        if (m_parent == nullptr)
        {
            setFromLocalMatrix(worldTransform);
            return;
        }

        const glm::mat4 localTransform = inverseOrIdentity(m_parent->worldMatrix()) * worldTransform;
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
        const glm::mat4 translation = glm::translate(glm::mat4{1.0f}, m_position);
        const glm::mat4 rotation = glm::mat4_cast(m_rotation);
        const glm::mat4 scaling = glm::scale(glm::mat4{1.0f}, m_scale);
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
