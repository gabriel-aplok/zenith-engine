#pragma once

#include "math/math.hpp"
#include "render/mesh.hpp"
#include "scene/component.hpp"

namespace Zenith::Components
{
    class Camera final : public Component
    {
    public:
        enum class ClearFlags
        {
            Skybox,
            SolidColor,
            DepthOnly,
            Nothing,
        };

        Camera() = default;
        ~Camera() override = default;

        float fieldOfView() const { return m_fieldOfView; }
        void setFieldOfView(float fieldOfView) { m_fieldOfView = fieldOfView; }

        float nearClipPlane() const { return m_nearClipPlane; }
        void setNearClipPlane(float nearClipPlane) { m_nearClipPlane = nearClipPlane; }

        float farClipPlane() const { return m_farClipPlane; }
        void setFarClipPlane(float farClipPlane) { m_farClipPlane = farClipPlane; }

        bool orthographic() const { return m_orthographic; }
        void setOrthographic(bool orthographic) { m_orthographic = orthographic; }

        float orthographicSize() const { return m_orthographicSize; }
        void setOrthographicSize(float orthographicSize) { m_orthographicSize = orthographicSize; }

        float depth() const { return m_depth; }
        void setDepth(float depth) { m_depth = depth; }

        ClearFlags clearFlags() const { return m_clearFlags; }
        void setClearFlags(ClearFlags clearFlags) { m_clearFlags = clearFlags; }

        const Vector4 &backgroundColor() const { return m_backgroundColor; }
        void setBackgroundColor(const Vector4 &backgroundColor) { m_backgroundColor = backgroundColor; }

        bool buildViewState(const Matrix4 &worldMatrix, const IVector2 &framebufferSize, Render::RenderViewState &outView) const;

    private:
        float m_fieldOfView = 60.0f;
        float m_nearClipPlane = 0.3f;
        float m_farClipPlane = 1000.0f;
        bool m_orthographic = false;
        float m_orthographicSize = 5.0f;
        float m_depth = 0.0f;
        ClearFlags m_clearFlags = ClearFlags::SolidColor;
        Vector4 m_backgroundColor{0.08f, 0.09f, 0.11f, 1.0f};
    };
} // namespace Zenith::Components
