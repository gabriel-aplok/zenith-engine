#include "components/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Zenith::Components
{
    bool Camera::buildViewState(const glm::mat4 &worldMatrix, const glm::ivec2 &framebufferSize, Render::RenderViewState &outView) const
    {
        const float aspect = framebufferSize.y > 0 ? static_cast<float>(framebufferSize.x) / static_cast<float>(framebufferSize.y) : 1.0f;

        outView.view = glm::inverse(worldMatrix);
        if (m_orthographic)
        {
            const float halfHeight = m_orthographicSize * 0.5f;
            const float halfWidth = halfHeight * aspect;
            outView.projection = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, m_nearClipPlane, m_farClipPlane);
        }
        else
        {
            outView.projection = glm::perspective(glm::radians(m_fieldOfView), aspect, m_nearClipPlane, m_farClipPlane);
            outView.projection[1][1] *= -1.0f;
        }

        return true;
    }
} // namespace Zenith::Components
