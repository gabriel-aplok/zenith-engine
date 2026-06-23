#include "engine/application.hpp"
#include "engine/startup.hpp"
#include "log/log.hpp"
#include "render/bgfx_renderer.hpp"
#include "render/irenderer.hpp"
#include "render/mesh_builder.hpp"
#include "render/mesh_cache.hpp"
#include "render/render_context.hpp"
#include "render/render_submission.hpp"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace Zenith
{

    class GameApplication final : public Application
    {
    public:
        explicit GameApplication(const ApplicationConfig &config)
            : Application(config) {}

    protected:
        void onInit() override
        {
            m_renderContext = createRenderContext(getWindow(), getConfig().debug);
            if (!m_renderContext)
            {
                Log::Error("Failed to create render context");
                requestQuit();
                return;
            }

            m_renderer = std::make_unique<Render::BgfxRenderer>();
            if (!m_renderer->initialize(*m_renderContext))
            {
                Log::Error("Failed to initialize renderer");
                requestQuit();
                return;
            }

            m_meshCache.setUploader(m_renderer.get());
            m_pyramidMesh = m_meshCache.acquireRef("demo/pyramid", MeshBuilder::makePyramid());
            m_cubeMesh = m_meshCache.acquireRef("demo/cube", MeshBuilder::makeCube());
            m_planeMesh = m_meshCache.acquireRef("demo/plane", MeshBuilder::makePlane());
            if (!m_pyramidMesh || !m_cubeMesh || !m_planeMesh)
            {
                Log::Error("Failed to create demo meshes");
                requestQuit();
            }
        }

        void onRender() override
        {
            if (!m_renderer || !m_renderContext)
            {
                return;
            }

            m_renderContext->beginFrame();
            m_frame.begin(3);
            m_frame.setView(makeViewState());
            updateDynamicPlane();
            m_frame.submitMesh(m_pyramidMesh.handle(), glm::translate(glm::mat4{1.0f}, glm::vec3{-1.5f, 0.0f, 0.0f}), {.tint = glm::vec4{1.0f}});
            m_frame.submitMesh(m_cubeMesh.handle(), glm::translate(glm::mat4{1.0f}, glm::vec3{1.5f, 0.0f, 0.0f}), {.tint = glm::vec4{0.9f, 1.0f, 1.0f, 1.0f}});
            m_frame.submitMesh(m_planeMesh.handle(), glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, -1.5f, 0.0f}), {.tint = glm::vec4{0.9f, 0.9f, 0.9f, 1.0f}});
            m_frame.finalize();
            m_renderer->render(m_frame);
            m_renderContext->endFrame();
        }

        void onShutdown() override
        {
            m_planeMesh.reset();
            m_cubeMesh.reset();
            m_pyramidMesh.reset();
            if (m_renderer)
            {
                m_meshCache.clear();
                m_renderer->shutdown();
            }

            m_renderer.reset();
            m_renderContext.reset();
        }

    private:
        void updateDynamicPlane()
        {
            m_phase += 0.02f;

            Render::MeshData plane = MeshBuilder::makePlane();
            const float xOffset = std::sin(m_phase) * 0.25f;
            const float yOffset = std::cos(m_phase * 0.7f) * 0.1f;

            for (auto &vertex : plane.vertices)
            {
                vertex.position.x += xOffset;
                vertex.position.y += yOffset;
            }

            if (!m_meshCache.update("demo/plane", plane))
            {
                Log::Error("Failed to update dynamic plane mesh");
            }
        }

        Render::RenderViewState makeViewState() const
        {
            const glm::vec3 eye{0.0f, 2.5f, 6.0f};
            const glm::vec3 target{0.0f, 0.0f, 0.0f};
            const glm::vec3 up{0.0f, 1.0f, 0.0f};
            const glm::ivec2 size = m_renderContext ? m_renderContext->framebufferSize() : glm::ivec2{1, 1};
            const float aspect = size.y > 0 ? static_cast<float>(size.x) / static_cast<float>(size.y) : 1.0f;

            Render::RenderViewState viewState{};
            viewState.view = glm::lookAt(eye, target, up);
            viewState.projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
            viewState.projection[1][1] *= -1.0f;
            return viewState;
        }

        std::unique_ptr<RenderContext> m_renderContext;
        std::unique_ptr<IRenderer> m_renderer;
        Render::RenderMeshCache m_meshCache;
        Render::MeshRef m_pyramidMesh{};
        Render::MeshRef m_cubeMesh{};
        Render::MeshRef m_planeMesh{};
        RenderFrame m_frame{};
        float m_phase = 0.0f;
    };

} // namespace Zenith

int main(int argc, char **argv)
{
    Zenith::Log::Init();
    Zenith::GameApplication app{Zenith::parseApplicationConfig(argc, argv)};
    app.run();
    return 0;
}
