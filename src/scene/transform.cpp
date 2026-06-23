#include "scene/transform.hpp"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

namespace Zenith
{
    Transform::Transform()
    {
        updateLocalMatrix();
        updateWorldMatrix();
    }

    void Transform::setPosition(const glm::vec3 &position)
    {
        m_position = position;
        markLocalDirty();
    }

    void Transform::setRotation(const glm::quat &rotation)
    {
        m_rotation = glm::normalize(rotation);
        markLocalDirty();
    }

    void Transform::setRotationEulerRadians(const glm::vec3 &eulerRadians)
    {
        setRotation(glm::quat(eulerRadians));
    }

    void Transform::setRotationEulerDegrees(const glm::vec3 &eulerDegrees)
    {
        setRotationEulerRadians(glm::radians(eulerDegrees));
    }

    void Transform::setScale(const glm::vec3 &scale)
    {
        m_scale = scale;
        markLocalDirty();
    }

    void Transform::setParent(Transform *parent)
    {
        if (m_parent == parent || parent == this)
        {
            return;
        }

        for (Transform *ancestor = parent; ancestor != nullptr; ancestor = ancestor->m_parent)
        {
            if (ancestor == this)
            {
                return;
            }
        }

        if (m_parent != nullptr)
        {
            auto &siblings = m_parent->m_children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        }

        m_parent = parent;
        if (m_parent != nullptr)
        {
            m_parent->m_children.push_back(this);
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

    void Transform::markLocalDirty()
    {
        m_localDirty = true;
        markWorldDirty();
    }

    void Transform::markWorldDirty()
    {
        if (!m_worldDirty)
        {
            m_worldDirty = true;
        }

        for (Transform *child : m_children)
        {
            child->markWorldDirty();
        }
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
        {
            updateLocalMatrix();
        }

        if (!m_worldDirty)
        {
            return;
        }

        if (m_parent != nullptr)
        {
            m_localToWorld = m_parent->worldMatrix() * m_localMatrix;
        }
        else
        {
            m_localToWorld = m_localMatrix;
        }

        m_worldDirty = false;
    }
} // namespace Zenith
