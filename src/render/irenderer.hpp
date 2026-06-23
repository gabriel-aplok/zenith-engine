#pragma once

#include <memory>

#include <glm/vec2.hpp>

#include "engine/window_backend.hpp"
#include "render/mesh_cache.hpp"
#include "render/mesh.hpp"
#include "render/render_submission.hpp"

namespace Zenith
{
    class RenderContext;

    class IRenderer : public Render::IMeshUploader
    {
    public:
        virtual ~IRenderer() = default;

        virtual bool initialize(RenderContext &context) = 0;
        virtual void shutdown() = 0;
        virtual void resize(const glm::ivec2 &framebufferSize) = 0;
        virtual void render(const RenderFrame &frame) = 0;
    };

    std::unique_ptr<IRenderer> createRenderer(GraphicsApi api);

} // namespace Zenith
